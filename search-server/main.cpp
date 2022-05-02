// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
		ASSERT(found_docs.size());
        const Document& doc0 = found_docs[0];
		ASSERT_EQUAL(doc0.id, doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

// Поддержка минус-слов. Документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
void TestMinusWords() {
	const int doc_id = 1;
	const string content = "cat in the city"s;
	const vector<int> ratings = {1, 2, 3};
	{
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("cat -in"s).empty());
        ASSERT(server.FindTopDocuments("-cat -in -the - city"s).empty());
        const auto found_docs = server.FindTopDocuments("in -dog"s);
        ASSERT_EQUAL(found_docs.size(), 1);
	}
}

// Матчинг документов


//Сортировка найденных документов по релевантности
void TestSortFindDocumentsByRelevance() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
	const auto found_docs = search_server.FindTopDocuments("пушистый ухоженный кот"s);
	ASSERT_EQUAL(found_docs[0].id, 1);
	ASSERT_EQUAL(found_docs[1].id, 0);
	ASSERT_EQUAL(found_docs[2].id,  2);
	ASSERT(found_docs[0].relevance >= found_docs[1].relevance);
	ASSERT(found_docs[1].relevance >= found_docs[2].relevance);
}

// Поиск документов, имеющих заданный статус.
void TestFindDocumentsStatus() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
	const auto found_docs = search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED);
	ASSERT_EQUAL(found_docs.size(), 1);
	const auto found_docs_2 = search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::REMOVED);
	ASSERT_EQUAL(found_docs_2.size(), 0);
	const auto found_docs_3 = search_server.FindTopDocuments("белый кот"s, DocumentStatus::ACTUAL);
	ASSERT_EQUAL(found_docs_3.size(), 2);
}

//Фильтрация результатов поиска с использованием предиката, задаваемого пользователем
void TestFilteringSearchResults() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
	const auto found_docs = search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0;});
	ASSERT_EQUAL(found_docs.size(), 2);
	const auto found_docs_2 = search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id == 0;});
	ASSERT_EQUAL(found_docs_2.size(), 1);
	const auto found_docs_3 = search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id == 10;});
	ASSERT_EQUAL(found_docs_3.size(), 0);

}

// Вычисление рейтинга документов
void TestCalculateRating() {
	SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
	const auto found_docs = search_server.FindTopDocuments("пушистый ухоженный кот"s);
	ASSERT_EQUAL(found_docs[0].rating, 5);
	ASSERT_EQUAL(found_docs[1].rating, 2);
}

// Корректное вычисление релевантности найденных документов
void TestCalculateRelevance() {
	SearchServer search_server;
    search_server.SetStopWords("и в"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
	const auto found_docs = search_server.FindTopDocuments("пушистый ухоженный кот"s);
	const double epsilon = 1e-6;
	ASSERT((found_docs[0].relevance - 0.866434) < epsilon);
	ASSERT((found_docs[1].relevance - 0.173287) < epsilon);

}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();
	TestMinusWords();
	TestSortFindDocumentsByRelevance();
	TestFindDocumentsStatus();
	TestFilteringSearchResults();
	TestCalculateRating();
	TestCalculateRelevance();
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------


