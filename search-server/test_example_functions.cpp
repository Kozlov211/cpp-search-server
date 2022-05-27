#include "test_example_functions.h"
#include "log_duration.h"
#include <vector>
#include <iostream>

using namespace std;

void AddDocument(SearchServer &search_server, int document_id, const std::string &document, DocumentStatus status, const std::vector<int> &ratings) {
    search_server.AddDocument (document_id, document, status, ratings);
}

void MatchDocuments(const SearchServer &search_server, const std::string &raw_query)
{
	{
		LOG_DURATION("Operation time"s, cout);
		cout << "Матчинг документов по запросу: "s << raw_query << endl;
		for (const int id : search_server) {
			const auto& [words, status] = search_server.MatchDocument(raw_query, id);
			cout << PrintMatchDocumentResult(id, words, status);
		}
	}
}

void FindTopDocuments(const SearchServer &search_server, const std::string &raw_query)
{
	{
		LOG_DURATION("Operation time"s, cout);
		cout << "Результаты поиска по запросу: "s << raw_query << endl;
		auto result = search_server.FindTopDocuments(raw_query);
		for (auto& document : result) {
			cout << document;
		}
	}
}
