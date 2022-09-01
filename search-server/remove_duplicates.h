#pragma once

#include <set>
#include <string>
#include <map>

#include "search_server.h"

void RemoveDuplicates(SearchServer& search_server);

std::set<std::string> AddWordToSet(std::map<std::string, double> word_frequencies);
