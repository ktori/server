add_executable(headers_test headers_test.c)
target_link_libraries(headers_test vhttpsl bits)

add_executable(request_line_test request_line_test.c)
target_link_libraries(request_line_test vhttpsl bits)

add_executable(uri_test uri_test.c)
target_link_libraries(uri_test vhttpsl bits)