#include "remove_duplicates.h"
#include <iostream>

using namespace std;

Original::Original(int id, map<string, double> word_frequencies) : id(id), word_frequencies(word_frequencies) {}

void RemoveDuplicates(SearchServer& search_server) {
	vector<Original> originals;
	for (const int document_id : search_server) {
		map<string, double> word_frequencies = search_server.GetWordFrequencies(document_id);
		if (originals.empty()) {
			originals.push_back({document_id, word_frequencies});
		} else {
			AddDuplicates(originals, word_frequencies, document_id);
		}
	}
	for (const Original& original : originals) {
		for (int id : original.duplicates) {
			search_server.RemoveDocument(id);
		}
	}
}

void AddDuplicates(vector<Original>& originals, const map<string, double>& word_frequencies, const int document_id) {
	for (Original& original : originals) {
		if (ComparisonKey(original.word_frequencies, word_frequencies)) {
			original.duplicates.push_back(document_id);
			return;
		}
	}
	originals.push_back({document_id, word_frequencies});
}

bool ComparisonKey(const map<string, double>& map_1, const map<string, double>& map_2) {
	if (map_1.size() != map_2.size()) {
		return false;
	}
	for (const auto [key, value] : map_1) {
		if (map_2.count(key) == 0) {
			return false;
		}
	}
	return true;
}
