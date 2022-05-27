#pragma once
#include "search_server.h"
#include <vector>
#include <map>

struct Original {
	Original(int id, std::map<std::string, double> word_frequencies);
	int id = 0;
	std::vector<int> duplicates;
	std::map<std::string, double> word_frequencies;
};

void RemoveDuplicates(SearchServer& search_server);

void AddDuplicates(std::vector<Original>& originals, const std::map<std::string, double>& word_frequencies, int document_id);

bool ComparisonKey(const std::map<std::string, double>& map_1, const std::map<std::string, double>& map_2);
