#include <algorithm>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <iostream>

#include "search_server.h"

using namespace std;

SearchServer::SearchServer(const string& stop_words_text) : SearchServer(SplitIntoWords(stop_words_text)) {}

SearchServer::SearchServer(string_view stop_words_text) : SearchServer(SplitIntoWords(stop_words_text)) {}

void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings) {
    if (documents_.count(document_id) != 0 || document_id < 0) {
        throw invalid_argument("Некорректный номер документа");
    }
    document_id_.insert(document_id);
    const vector<string_view> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    for (const string_view& word : words) {
        word_frequencies[document_id][string(word)] += inv_word_count;
        word_to_document_freqs_[(word_frequencies[document_id].find(word))->first][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                            return document_status == status; });
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const {
    return FindTopDocuments(raw_query,  DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(string_view raw_query, int document_id) const {
	if (!document_id_.count(document_id)) {
		throw std::out_of_range("Некорректный номер документа");
	}
	const Query query = ParseQuery(raw_query);
	if (any_of(query.minus_words.begin(), query.minus_words.end(), [this, document_id](const string_view& word) {
		return (word_to_document_freqs_.at(word).count(document_id) ? true : false);
	})) {
		return {vector<string_view> {}, documents_.at(document_id).status};
	}
	vector<string_view> matched_words(query.plus_words.size());
	auto it = copy_if(query.plus_words.begin(), query.plus_words.end(), matched_words.begin(), [this, document_id](const std::string_view& word) {
		return (word_to_document_freqs_.at(word).count(document_id));
	});
	matched_words.resize(distance(matched_words.begin(), it));
	set<string_view> unique_words(matched_words.begin(), matched_words.end());
	return {vector<string_view>(unique_words.begin(), unique_words.end()), documents_.at(document_id).status};
}

set<int>::const_iterator SearchServer::begin() const {
    return document_id_.begin();
}

set<int>::const_iterator SearchServer::end() const {
    return document_id_.end();
}

const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    if (word_frequencies.count(document_id) != 0) {
        return (map<std::string_view, double>&) word_frequencies.at(document_id);
    }
    static const map<string_view, double> dummy;
    return dummy;
}

void SearchServer::RemoveDocument (int document_id) {
    if (!document_id_.count(document_id)) {
        throw invalid_argument("Некорректный номер документа"s);
    }
    for (const auto& [word, freq] : word_frequencies.at(document_id)) {
            word_to_document_freqs_.at(word).erase(document_id);
    }
    documents_.erase(document_id);
    document_id_.erase(document_id);
    word_frequencies.erase(document_id);
}

bool SearchServer::IsStopWord(string_view word) const {
    return stop_words_.count(word) > 0;
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(string_view text) const {
    vector<string_view> words;
    for (const string_view& word : SplitIntoWords(text)) {
        IsValidWord(word);
        if(!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

bool SearchServer::IsValidWord(string_view word) {
    if (none_of(word.begin(), word.end(), [](char a) {
                return a >= '\0' && a < ' ';})) {
        return true;
    }
    throw invalid_argument("Наличие недопустимых символов в тексте запроса");
}

bool SearchServer::IsValidMinusWord(string_view word) {
    if (!(word.empty() || word[0] == '-')) {
        return true;
    }
    throw std::invalid_argument("Наличие недопустимых минус слов");
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {
	IsValidWord(text);
	bool is_minus = false;
	if (text[0] == '-') {
		is_minus = true;
		text = text.substr(1);
	}
	IsValidMinusWord(text);
	return {text, is_minus, IsStopWord(text)};
}

SearchServer::Query SearchServer::ParseQuery(string_view text, bool is_policy) const {
    Query query;
	for (const string_view& word : SplitIntoWords(text)) {
		const QueryWord query_word = ParseQueryWord(word);
		if (!query_word.is_stop) {
			if (query_word.is_minus) {
				query.minus_words.push_back(query_word.data);
			} else {
				query.plus_words.push_back(query_word.data);
			}
		}
	}
	std::sort(query.minus_words.begin(), query.minus_words.end());
	auto last = std::unique(query.minus_words.begin(), query.minus_words.end());
	query.minus_words.erase(last, query.minus_words.end());
	std::sort(query.plus_words.begin(), query.plus_words.end());
	last = std::unique(query.plus_words.begin(), query.plus_words.end());
	query.plus_words.erase(last, query.plus_words.end());
	return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(string_view word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

