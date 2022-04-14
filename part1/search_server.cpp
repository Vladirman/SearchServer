#include "search_server.h"


#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <numeric>
vector<string> SplitIntoWords(const string& line) {
  istringstream words_input(line);
  return {istream_iterator<string>(words_input), istream_iterator<string>()}; 
}

SearchServer::SearchServer(istream& document_input) {
  UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream& document_input) {
  InvertedIndex new_index;

  for (string current_document; getline(document_input, current_document); ) {
    new_index.Add(move(current_document));
  }

  index = move(new_index);
}

void SearchServer::AddQueriesStream(
  istream& query_input, ostream& search_results_output
) {
	for (string word; getline(query_input, word);) {
		vector<pair<size_t, size_t>> result(index.Doc_sz());
		for (const auto& c : SplitIntoWords(word)) {
			for (auto& r : index.Lookup(c)) { 
				result[r.first] = { r.first, result[r.first].second + r.second };
			}
		}
		if (result.size() >= 5) {
			partial_sort(result.begin(), result.begin() + 5, result.end(), [](pair<size_t, size_t> lhs, pair<size_t, size_t> rhs) {
				int64_t lhs_docid = lhs.first;
				auto lhs_hit_count = lhs.second;
				int64_t rhs_docid = rhs.first;
				auto rhs_hit_count = rhs.second;
				return make_pair(lhs_hit_count, -lhs_docid) > make_pair(rhs_hit_count, -rhs_docid);
			});
		}
		else {
			sort(result.begin(), result.end(), [](pair<size_t, size_t> lhs, pair<size_t, size_t> rhs) {
				int64_t lhs_docid = lhs.first;
				auto lhs_hit_count = lhs.second;
				int64_t rhs_docid = rhs.first;
				auto rhs_hit_count = rhs.second;
				return make_pair(lhs_hit_count, -lhs_docid) > make_pair(rhs_hit_count, -rhs_docid);
			});
		}
		search_results_output << word << ':';
		if (result.size() >= 5)
		{
			for (size_t i = 0; i < 5; i++) {
				if (result[i].second != 0)
				{
					search_results_output << " {"
						<< "docid: " << result[i].first << ", "
						<< "hitcount: " << result[i].second << '}';
				}
			}
		}
		else {
			for (size_t i = 0; i < result.size(); i++) {
				if (result[i].second != 0)
				{
					search_results_output << " {"
						<< "docid: " << result[i].first << ", "
						<< "hitcount: " << result[i].second << '}';
				}
			}
		}
		search_results_output << endl;
		
	}
}

void InvertedIndex::Add(const string& document) {
	set<string> tmp;
	for (auto& c : SplitIntoWords(document)) {
		auto it = tmp.insert(c);
		if (it.second == true) {
			result[c].push_back({ docs, 1 });
		}
		else {
			size_t size = result[c].size() - 1;
			size_t cap = result[c][size].second;
			
			swap(result[c][size].second, ++cap);
		}
	}
	++docs;
}

const vector<pair<size_t, size_t>>& InvertedIndex::Lookup(const string& word) const {
	auto it = result.find(word);
	static vector<pair<size_t, size_t>> empty_vec;
	if (it != result.end())
	{
		return it->second;
	}
	else return empty_vec;
}
