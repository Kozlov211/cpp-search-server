#include "remove_duplicates.h"
#include <iostream>
#include <map>

using namespace std;


void RemoveDuplicates(SearchServer& search_server) {
	set<set<string>> originals;
	vector<int> duplicates;
	set<string> words;
	map<string, double> word_frequencies;
	for (const int document_id : search_server) {
		word_frequencies = search_server.GetWordFrequencies(document_id);
		words = AddWordToSet(word_frequencies);
		if (originals.count(words) != 0) {
			duplicates.push_back(document_id);
		} else {
			originals.insert(words);
		}
	}
	for (const int id : duplicates) {
		cout << "Found duplicate document id " << id << endl;
		search_server.RemoveDocument(id); 
	}
}

set<string> AddWordToSet(map<string, double> word_frequencies) {
	set<string> result;
	for (const auto [word, freq] : word_frequencies) {
		result.insert(word);
	}
	return result;
}

