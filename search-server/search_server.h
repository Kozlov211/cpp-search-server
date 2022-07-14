#pragma once
#include <string>
#include <set>
#include <vector>
#include <tuple>
#include <stdexcept>
#include <algorithm>
#include <execution>
#include <thread>
#include <mutex>
#include <map>
#include <string>
#include <string_view>

#include "concurrent_map.h"
#include "document.h"
#include "string_processing.h"


const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

class SearchServer {
public:
	template <typename StringContainer>
	explicit SearchServer(const StringContainer& stop_words);

	explicit SearchServer(const std::string& stop_words_text);

	explicit SearchServer(std::string_view stop_words_text);

	void AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings);

	template <typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentPredicate document_predicate) const;
	template <typename ExecutionPolicy, typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(ExecutionPolicy policy, std::string_view raw_query, DocumentPredicate document_predicate) const;
	template <typename ExecutionPolicy>
	std::vector<Document> FindTopDocuments(ExecutionPolicy policy, std::string_view raw_query, DocumentStatus status) const;
	template <typename ExecutionPolicy>
	std::vector<Document> FindTopDocuments(ExecutionPolicy policy, std::string_view raw_query) const;
	std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentStatus status) const;
	std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

	int GetDocumentCount() const;

	template <typename ExecutionPolicy>
	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy policy, std::string_view raw_query, int document_id) const;
	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query, int document_id) const;

	std::set<int>::const_iterator begin() const;
	std::set<int>::const_iterator end() const;

	const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

	template <typename ExecutionPolicy>
	void RemoveDocument(ExecutionPolicy&& policy, int document_id);
	void RemoveDocument(int document_id);

private:
	struct DocumentData {
		int rating;
		DocumentStatus status;
	};

	const std::set<std::string, std::less<>> stop_words_;
	std::map<std::string_view, std::map<int, double>, std::less<>> word_to_document_freqs_;
	std::map<int, DocumentData> documents_;
	std::set<int> document_id_;
	std::map<int, std::map<std::string, double, std::less<>>> word_frequencies;
	std::vector<std::string> words;

	bool IsStopWord(std::string_view word) const;

	std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;

	static bool IsValidWord(std::string_view word);

	static bool IsValidMinusWord(std::string_view word);

	static int ComputeAverageRating(const std::vector<int>& ratings);

	struct QueryWord {
		std::string_view data;
		bool is_minus;
		bool is_stop;
	};

	QueryWord ParseQueryWord(std::string_view text) const;

	struct Query {
	std::vector<std::string_view> plus_words;
	std::vector<std::string_view> minus_words;
	};

	Query ParseQuery(std::string_view text, bool is_policy = false) const;

	double ComputeWordInverseDocumentFreq(std::string_view word) const;

	template <typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;

	template <typename ExecutionPolicy, typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(ExecutionPolicy policy, const Query& query, DocumentPredicate document_predicate) const;

};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words) : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
    for (const std::string_view& word : stop_words_) {
        IsValidWord(word);
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentPredicate document_predicate) const {
	return FindTopDocuments(std::execution::seq, raw_query, document_predicate);
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy policy, std::string_view raw_query, DocumentPredicate document_predicate) const {
	const Query query = ParseQuery(raw_query, true);
	std::vector<Document> matched_documents = FindAllDocuments(policy, query, document_predicate);
	sort(policy, matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
	if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
		return lhs.rating > rhs.rating;
	} else {
		return lhs.relevance > rhs.relevance;
	} });
	if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
		matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
	}
	return matched_documents;
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy policy, std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                            return document_status == status;});
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy policy, std::string_view raw_query) const {
    return FindTopDocuments(policy, raw_query,  DocumentStatus::ACTUAL);
}


template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
	std::map<int, double> document_to_relevance;
	for (const std::string_view& word : query.plus_words) {
		IsValidWord(word);
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
		for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
			const auto& document_data = documents_.at(document_id);
			if (document_predicate(document_id, document_data.status, document_data.rating)) {
				document_to_relevance[document_id] += term_freq * inverse_document_freq;
			}
		}
	}
	for (const std::string_view& word : query.minus_words) {
		IsValidMinusWord(word);
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
			document_to_relevance.erase(document_id);
		}
	}
	std::vector<Document> matched_documents;
	for (const auto [document_id, relevance] : document_to_relevance) {
	matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
	}
	return  matched_documents;
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(ExecutionPolicy policy, const Query& query, DocumentPredicate document_predicate) const {
	ConcurrentMap<int, double> document_to_relevance(5000);
	std::for_each(policy, query.plus_words.begin(), query.plus_words.end(), [this, &document_to_relevance, &document_predicate] (const std::string_view& word) {
		IsValidWord(word);
		if (word_to_document_freqs_.count(word) != 0) {
			const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
			auto tmp_map = word_to_document_freqs_.at(std::string(word));
			std::for_each(tmp_map.begin(), tmp_map.end(), [this, &document_predicate, &document_to_relevance, &inverse_document_freq](const std::pair<int, double>& tmp) {
				const auto& document_data = documents_.at(tmp.first);
				if (document_predicate(tmp.first, document_data.status, document_data.rating)) {
				document_to_relevance[tmp.first].ref_to_value += tmp.second * inverse_document_freq;
			}});
		}
	});
	std::for_each(policy, query.minus_words.begin(), query.minus_words.end(), [this, &document_to_relevance, &document_predicate] (std::string_view word) {
		IsValidMinusWord(word);
		if (word_to_document_freqs_.count(word) != 0) {
			for (const auto [document_id, _] : word_to_document_freqs_.at(std::string(word))) {
				document_to_relevance.Erase(document_id);
			}
		}
	});
	std::vector<Document> matched_documents;
	for (const auto [document_id, relevance] : document_to_relevance.BuildOrdinaryMap()) {
		matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
	}
	return  matched_documents;
}

template <typename ExecutionPolicy>
void SearchServer::RemoveDocument(ExecutionPolicy&& policy, int document_id) {
	if (!document_id_.count(document_id)) {
		throw std::out_of_range("Некорректный номер документа");
	}
	std::map<std::string_view, double>& tmp_map = word_frequencies[document_id];
	std::vector<std::map<std::string_view, double>::iterator> it_map;
	it_map.reserve(tmp_map.size());
	for (auto it = tmp_map.begin(); it != tmp_map.end(); ++it) {
		it_map.push_back(it);
	}
	std::for_each(policy, it_map.begin(), it_map.end(), [this, document_id](std::map<std::string_view, double>::iterator& tmp) {
		word_to_document_freqs_[std::string((*tmp).first)].erase(document_id);});
	documents_.erase(document_id);
	document_id_.erase(document_id);
	word_frequencies.erase(document_id);
}

template <typename ExecutionPolicy>
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy policy, std::string_view raw_query, int document_id) const {
	if (!document_id_.count(document_id)) {
		throw std::out_of_range("Некорректный номер документа");
	}
	if (std::is_same<ExecutionPolicy, std::execution::sequenced_policy>::value) {
		return MatchDocument(raw_query, document_id);
	}
	Query query = ParseQuery(policy, raw_query);
	if (any_of(policy, query.minus_words.begin(), query.minus_words.end(), [this, document_id](const std::string_view& word) {
		return (word_to_document_freqs_.at(word).count(document_id) ? true : false);
	})) {
		return {{}, documents_.at(document_id).status};
	}
	std::vector<std::string_view> matched_words(query.plus_words.size());
	auto it = std::copy_if(policy, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(), [this, document_id](const std::string_view& word) {
	return (word_to_document_freqs_.at(word).count(document_id));
	});
	matched_words.resize(std::distance(matched_words.begin(), it));
	std::set<std::string_view> unique_words(matched_words.begin(), matched_words.end());
	return {std::vector<std::string_view>(unique_words.begin(), unique_words.end()), documents_.at(document_id).status};
}

