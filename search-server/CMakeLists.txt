cmake_minimum_required(VERSION 3.5)
project(SearchServer)

set(CMAKE_CXX_STANDARD 17)

set(MAIN_FILE main.cpp)
set(SEARCH_SERVER_FILES search_server.h search_server.cpp)
set(REQUES_QUEUE_FILES request_queue.h request_queue.cpp)
set(STRING_PROCESSING_FILES string_processing.h string_processing.cpp)
set(DOCUMENT_FILES document.h document.cpp)
set(READ_INPUT_FUNCTIONS_FILES read_input_functions.h read_input_functions.cpp)
set(REMOVE_DUPLICATES_FILES remove_duplicates.h remove_duplicates.cpp)
set(LOG_DURATION log_duration.h)
set(CONCURRENT_MAP concurrent_map.h)
set(PAGINATOR paginator.h)

add_executable(SearchServer ${MAIN_FILE} ${SEARCH_SERVER_FILES} ${REQUES_QUEUE_FILES}
							${STRING_PROCESSING_FILES} ${DOCUMENT_FILES} ${READ_INPUT_FUNCTIONS_FILES}
							${REMOVE_DUPLICATES_FILES} ${LOG_DURATION} ${CONCURRENT_MAP} ${PAGINATOR})

