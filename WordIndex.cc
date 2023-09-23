#include "./WordIndex.h"
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <algorithm>
#include <queue>

// #include <list>
using namespace std;

// using std::cout;
// using std::endl;
namespace searchserver {

WordIndex::WordIndex() {
  // TODO: implement
  
}

size_t WordIndex::num_words() {
  // TODO: implement
  return map.size();
}

void WordIndex::record(const string& word, const string& doc_name) {
  // TODO: implement
  map[word][doc_name]++;
}

vector<Result> WordIndex::lookup_word(const string& word) {
  // TODO: implement
  vector<Result> result_vector;
  list<Result> doc_list;
  for (const auto& [doc_name, count] : map[word]) {
    doc_list.push_back(Result(doc_name, count));
  }
  doc_list.sort();
  std::copy(doc_list.begin(), doc_list.end(), std::back_inserter(result_vector));
  return result_vector;
}

vector<Result> WordIndex::lookup_query(const vector<string>& query) {
  // TODO: implement
  vector<Result> query_result;
  list<Result> doc_list;
  if(query.size() == 0||map.find(query[0]) == map.end()) return query_result;
  for (const auto& it : map[query[0]]){
    string doc_name = it.first;
    int total_count = 0;
    for(size_t i = 0; i < query.size(); i++){
      
      if (map[query[i]].find(doc_name) != map[query[i]].end()){
        total_count += map[query[i]][doc_name];
      } else{
        break;
      }
      if (i == query.size()- 1){
        doc_list.push_back(Result(doc_name, total_count));
      }
    }
  }
  doc_list.sort();
  std::copy(doc_list.begin(), doc_list.end(), std::back_inserter(query_result));
  return query_result;
}

}  // namespace searchserver
