#include "request_queue.h"

using namespace std;

void RequestQueue::AddRequest(vector<Document> result) {
	if (real_time > min_in_day_) {
		count_empty_request_ -= requests_.front().type;
		requests_.pop_front();
		requests_.push_back({result, result.empty()});
		count_empty_request_ += requests_.back().type;
	} else {
		requests_.push_back({result, result.empty()});
		count_empty_request_ += requests_.back().type;
	}
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
	++real_time;
	vector<Document> result = search_server_.FindTopDocuments(raw_query, status);
	AddRequest(result);
	return result;
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
	++real_time;
	vector<Document> result = search_server_.FindTopDocuments(raw_query);
	AddRequest(result);
	return result;
}

int RequestQueue::GetNoResultRequests() const {
	return count_empty_request_;
}

