#include "document.h"
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

Document::Document(int id, double relevance, int rating) : id(id), relevance(relevance), rating(rating) {}

ostream& operator<<(ostream& output, Document document) {
	output << "{ " << "document_id = " << document.id << ", " << "relevance = " << document.relevance << ", " << "rating = " << document.rating << " }";
	return output;
}

string PrintDocumentToString(const Document& document) {
	ostringstream out;
	out << document;
	return out.str();
}

string PrintMatchDocumentResult(int id, vector<string> words, DocumentStatus status) {
	ostringstream out;
	out << "{ " << "document_id= " << id << ", " << "status = " << static_cast<int>(status) << ", " << "words = ";
	for (const string word : words) {
		out << word << " ";
	}
	out << endl;
	return out.str();
}

