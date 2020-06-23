add_executable(headers_test headers_test.c)
target_link_libraries(headers_test vhttpsl bits)
add_test(NAME headers COMMAND headers_test)

add_executable(request_line_test request_line_test.c)
target_link_libraries(request_line_test vhttpsl bits)
add_test(NAME request_line COMMAND request_line_test)

add_executable(uri_test uri_test.c)
target_link_libraries(uri_test vhttpsl bits)
add_test(NAME uri COMMAND uri_test)