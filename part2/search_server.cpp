#include "search_server.h"
#include "iterator_range.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <numeric>

string_view Strips(string_view s) {
	while (!s.empty() && isspace(s.front())) {
		s.remove_prefix(1);
	}
	while (!s.empty() && isspace(s.back())) {
		s.remove_suffix(1);
	}
	return s;
}

vector<string_view> SplitIntoWords(string_view line) {
	vector<string_view> result;
	size_t pos = 0;
	string_view str = Strips(line);
	const size_t pos_end = str.npos;
	while (true) {
		size_t tmp = 0;
		size_t space = str.find(' ', pos);
		while (str.at(space + 1) == ' ') {
			++space;
			++tmp;
		}
		result.push_back(space == pos_end ?
			str.substr(pos) :
			str.substr(pos, space - pos - tmp));
		if (space == pos_end) {
			break;
		}
		else {
			pos = space + 1;
		}
	}
	return result;
}

void SearchServer::UpdateDocumentBaseSingleThread(istream& document_input) {
	InvertedIndex new_index(document_input);

	swap(index.GetAccess().ref_to_value, new_index);
}

void SearchServer::UpdateDocumentBase(istream& document_input) {
	futures.push_back(async(&SearchServer::UpdateDocumentBaseSingleThread, this,
		ref(document_input))
	);
}
void SearchServer::AddQueriesStreamSingleThread(
	istream& query_input, ostream& search_results_output
) {
	vector<pair<size_t, size_t>> result;
	for (string word; getline(query_input, word);) {
		{
			const auto& access = index.GetAccess();
			auto& lock_idx = access.ref_to_value;
			result.assign(lock_idx.Doc_sz(), { 0,0 });
			for (const auto& s : SplitIntoWords(word)) {
				for (auto& r : lock_idx.Lookup(string(s))) {
					result[r.first] = { r.first, result[r.first].second + r.second };
				}
			}
		}
		partial_sort(result.begin(), Head(result, 5).end(), result.end(), [](pair<size_t, size_t> lhs, pair<size_t, size_t> rhs) {
			int64_t lhs_docid = lhs.first;
			auto lhs_hit_count = lhs.second;
			int64_t rhs_docid = rhs.first;
			auto rhs_hit_count = rhs.second;
			return make_pair(lhs_hit_count, -lhs_docid) > make_pair(rhs_hit_count, -rhs_docid);
		});
		search_results_output << word << ':';
		for (auto&[first, second] : Head(result, 5)) {
			if (second != 0)
			{
				search_results_output << " {"
					<< "docid: " << first << ", "
					<< "hitcount: " << second << '}';
			}
		}
		search_results_output << endl;
	}
}

void SearchServer::AddQueriesStream(istream& query_input, ostream& search_results_output
) {
	futures.push_back(async(&SearchServer::AddQueriesStreamSingleThread, this,
		ref(query_input), ref(search_results_output))
	);
}

void InvertedIndex::Add(const string& document) {
	for (auto& s : SplitIntoWords(document)) { 
		string c(s);
		auto& vec = result[c];
		if (vec.empty() || vec.back().first != docs) {
			vec.push_back({ docs, 1 });
		}
		else {
			size_t size = vec.size() - 1;
			size_t cap = vec[size].second;

			swap(vec[size].second, ++cap);
		}
	}
	++docs;
}

const vector<pair<size_t, size_t>>& InvertedIndex::Lookup(const string& word) const {
	auto it = result.find(word);
	if (it != result.end())
	{
		return it->second;
	}
	else return empty_vec;
}

InvertedIndex::InvertedIndex(istream& stream) {
	for (string current_document; getline(stream, current_document); ) {
		Add(current_document);
	}
}