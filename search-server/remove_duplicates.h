#pragma once
#include "search_server.h"
#include <set>
#include <string>
#include <map>

void RemoveDuplicates(SearchServer& search_server);

std::set<std::string> AddWordToSet(std::map<std::string, double> word_frequencies);
