#include "document.h"
#include <iostream>
#include <sstream>

using namespace std;

Document::Document(int id, double relevance, int rating) : id(id), relevance(relevance), rating(rating) {}

ostream& operator<<(ostream& output, Document document) {
       output << "{ "
         << "document_id = " << document.id << ", "
         << "relevance = " << document.relevance << ", "
         << "rating = " << document.rating << " }";
    return output;
}

void PrintDocument(const Document& document) {
    cout << document;
}

string PrintDocumentToString(const Document& document) {
    ostringstream out;
    out << document;
    return out.str();
}

