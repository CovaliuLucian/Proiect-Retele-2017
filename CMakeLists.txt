cmake_minimum_required(VERSION 3.9)
project(Proiect_Retele)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -L/usr/lib -lssl -lcrypto -lsqlite3")

link_libraries(crypto)
link_libraries(sqlite3)
link_libraries(ssl)

add_executable(client
        client.cpp
        operand.h
        operator.h
        request.h
        response.h)

add_executable(server
        operand.h
        operator.h
        parsing.h
        request.h
        response.h
        server.cpp
        token.h
        tree.h)

add_executable(XML
        XMLCreate.cpp)
