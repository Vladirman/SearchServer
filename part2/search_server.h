#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <vector>
#include <map>
#include <string>
#include <future>
#include <mutex>
#include <deque>
#include <unordered_map>
using namespace std;

template <typename T>
class Synchronized {
public:
	explicit Synchronized(T initial = T())
		: value(move(initial))
	{
	}

	struct Access {
		lock_guard<mutex> guard;
		T& ref_to_value;
	};

	Access GetAccess() {
		return {  lock_guard(m), value };
	}

private:
	T value;
	mutex m;
};

class InvertedIndex {
public:
	InvertedIndex() = default;
	explicit InvertedIndex(istream& stream);

	void Add(const string& document);
	const vector<pair<size_t, size_t>>& Lookup(const string& word) const;

	const size_t Doc_sz() const {
		return docs;
	}
	inline static vector<pair<size_t, size_t>> empty_vec = {};

private:
	unordered_map<string, vector<pair<size_t, size_t>>> result;
	size_t docs = 0;
};

class SearchServer {
public:
	SearchServer() = default;
	explicit SearchServer(istream& document_input) : index(InvertedIndex(document_input)) {};

	void UpdateDocumentBaseSingleThread(istream& document_input);
	void UpdateDocumentBase(istream& document_input);

	void AddQueriesStreamSingleThread(istream& query_input, ostream& serach_result_output);
	void AddQueriesStream(istream& query_input, ostream& search_results_output);


private:
	Synchronized<InvertedIndex> index;
	vector<future<void>> futures;
};