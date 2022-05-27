#include "search_server.h"
#include <algorithm>
#include <numeric>
#include <cmath>

using namespace std;

SearchServer::SearchServer(const string& stop_words_text) : SearchServer(SplitIntoWords(stop_words_text)) {}

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
	if (documents_.count(document_id) != 0 || document_id < 0) {
		throw invalid_argument("Некорректный номер документа");
	}
	document_id_.insert(document_id);
	const vector<string> words = SplitIntoWordsNoStop(document);
	const double inv_word_count = 1.0 / words.size();
	for (const string& word : words) {
		word_to_document_freqs_[word][document_id] += inv_word_count;
        word_frequencies[document_id][word] += inv_word_count;
	}
	documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus status) const {
	return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
							return document_status == status; });
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
	return FindTopDocuments(raw_query,  DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
	return documents_.size();
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
	const Query query = ParseQuery(raw_query);
	vector<string> matched_words;
	for (const string& word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
		    continue;
		}
		if (word_to_document_freqs_.at(word).count(document_id)) {
		    matched_words.push_back(word);
		}
	}
	for (const string& word : query.minus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
		    continue;
		}
		if (word_to_document_freqs_.at(word).count(document_id)) {
		    matched_words.clear();
		    break;
		}
	}
	return make_tuple(matched_words, documents_.at(document_id).status);
}

set<int>::const_iterator SearchServer::begin() const {
	return document_id_.begin();
}
    
set<int>::const_iterator SearchServer::end() const {
	return document_id_.end();
}

const map<string, double>& SearchServer::GetWordFrequencies(int document_id) const {
	if (word_frequencies.count(document_id) != 0) {
		return word_frequencies.at(document_id);
	}
	return word_frequencies_empty;
}

void SearchServer::RemoveDocument (int document_id) {
	documents_.erase(document_id);
	document_id_.erase(document_id);
	word_frequencies.erase(document_id);
	for (auto& [word, document] : word_to_document_freqs_) {
		if (document.count(document_id) != 0) {
			document.erase(document_id);
		}
	}
}

bool SearchServer::IsStopWord(const string& word) const {
	return stop_words_.count(word) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
	vector<string> words;
	for (const string& word : SplitIntoWords(text)) {
		if (!IsValidWord(word)) {
			throw invalid_argument("Наличие недопустимых символов в тексте добавляемого документа");
		}
		if (!IsStopWord(word)) {
		    words.push_back(word);
		}
	}
return words;
}

bool SearchServer::IsValidWord(const string& word) {
	return none_of(word.begin(), word.end(), [](char a) {
		 	    return a >= '\0' && a < ' ';});
}

bool SearchServer::IsValidMinusWord(const string& word) {
	if (word.empty() || word[0] == '-') {
		return false;
	}
	return true;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
	if (ratings.empty()) {
		return 0;
	}
	int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
	return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {
	bool is_minus = false;
	if (text[0] == '-') {
		is_minus = true;
		text = text.substr(1);
	}
	return {text, is_minus, IsStopWord(text)};
}

SearchServer::Query SearchServer::ParseQuery(const string& text) const {
	Query query;
	for (const string& word : SplitIntoWords(text)) {
		const QueryWord query_word = ParseQueryWord(word);
		if (!query_word.is_stop) {
		    if (!IsValidWord(query_word.data)) {
		        throw invalid_argument("Наличие недопустимых символов в тексте запроса");
		    }
		    if (!IsValidMinusWord(query_word.data)) {
		        throw invalid_argument("Наличие недопустимых минус слов");
		    }
		    if (query_word.is_minus) {
		        query.minus_words.insert(query_word.data);
		    } else {
		        query.plus_words.insert(query_word.data);
		    }
		}
	}
	return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
	return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
