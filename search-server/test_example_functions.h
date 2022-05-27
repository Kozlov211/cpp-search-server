#pragma once
#include "search_server.h"
#include "document.h"
#include <string>

void AddDocument(SearchServer &search_server, int document_id, const std::string &document, DocumentStatus status = DocumentStatus::ACTUAL, const std::vector<int> &ratings = {});

void MatchDocuments(const SearchServer &search_server, const std::string &raw_query);

void FindTopDocuments(const SearchServer &search_server, const std::string &raw_query);
