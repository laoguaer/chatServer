aux_source_directory(. SC_LIST)
add_executable(ChatClient ${SC_LIST})

target_link_libraries(ChatClient pthread)