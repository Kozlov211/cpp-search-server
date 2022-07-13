#include "string_processing.h"
#include <iostream>

using namespace std;

vector<string_view> SplitIntoWords(string_view str) {
	vector<string_view> result;
	str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
	while (!str.empty()) {
		auto space = str.find(' ');
		result.push_back(space == str.npos ? str.substr(0, str.npos) : str.substr(0, space));
		str.remove_prefix(std::min(space, str.size()));
		str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
	}
	return result;
}

