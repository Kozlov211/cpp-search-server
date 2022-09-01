#pragma once

#include "search_server.h"
#include <vector>
#include <deque>

class RequestQueue {
public:
	explicit RequestQueue(const SearchServer& search_server) : search_server_ (search_server) {};

	template <typename DocumentPredicate>
	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

	std::vector<Document> AddFindRequest(const std::string& raw_query);

	int GetNoResultRequests() const;
    
private:
	struct QueryResult {
		std::vector<Document> result;
		bool type = false;
	};

	std::deque<QueryResult> requests_;
	const static int min_in_day_ = 1440;
	time_t real_time = 0;
	int count_empty_request_ = 0;
	const SearchServer& search_server_;

private:
	void AddRequest(std::vector<Document> result);
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
	++real_time;
	std::vector<Document> result = search_server_.FindTopDocuments(raw_query, document_predicate);
	AddRequest(result);
	return result;
}
