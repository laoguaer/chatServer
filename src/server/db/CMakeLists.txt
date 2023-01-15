aux_source_directory(. SC_LIST)
add_executable(DB_TEST ${SC_LIST})

target_link_libraries(DB_TEST mymuduo pthread mysqlclient)