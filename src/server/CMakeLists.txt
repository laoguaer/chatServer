aux_source_directory(. SRC_LIST)
#aux_source_directory(./db DB_LIST)
set(DB_LIST ./db/connectionpool.cpp ./db/db.cpp)
aux_source_directory(./modle MODLE_LIST)
aux_source_directory(./redis REDIS_LIST)

add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODLE_LIST} ${REDIS_LIST})

target_link_libraries(ChatServer mymuduo pthread mysqlclient hiredis)
add_subdirectory(db)