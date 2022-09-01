#include <iostream>

#include "search_server.h"
#include "request_queue.h"
#include "paginator.h"
#include "remove_duplicates.h"
#include "log_duration.h"

using namespace std;

int main() {
    SearchServer search_server("и в на"s);

    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2});
    search_server.AddDocument(3, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2});
    search_server.AddDocument(5, "большой пёс скворец евгений"s, DocumentStatus::ACTUAL, {1, 1, 1});

    vector<Document> result = search_server.FindTopDocuments("пушистый"s);
    for (auto& item : result) {
        std::cout << item << std::endl;
    }
}
