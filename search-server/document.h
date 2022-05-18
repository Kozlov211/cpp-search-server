#pragma once
#include <string>
#include <ostream>

struct Document {
	Document() = default;
	Document(int id, double relevance, int rating);
	int id = 0;
	double relevance = 0.0;
	int rating = 0;
};

enum class DocumentStatus {
	ACTUAL,
	IRRELEVANT,
	BANNED,
	REMOVED,
};

std::ostream& operator<<(std::ostream& output, Document document);

std::string PrintDocumentToString(const Document& document);

