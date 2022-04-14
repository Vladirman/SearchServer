#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <vector>
#include <map>
#include <string>
using namespace std;

class InvertedIndex {
public:
  void Add(const string& document);
  const vector<pair<size_t, size_t>>& Lookup(const string& word) const;

  const size_t Doc_sz() const {
	  return docs;
  }

private:
  map<string, vector<pair<size_t, size_t>>> result;
  size_t docs = 0;
};

class SearchServer {
public:
  SearchServer() = default;
  explicit SearchServer(istream& document_input); 
  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);
private:
  InvertedIndex index;
};
