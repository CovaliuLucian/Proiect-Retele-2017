cmake_minimum_required(VERSION 3.9)
project(Proiect_Retele)

set(CMAKE_CXX_STANDARD 11)

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
