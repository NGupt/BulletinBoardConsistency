cmake_minimum_required(VERSION 3.9)
project(BulletinBoardConsistency)

set(CMAKE_CXX_STANDARD 11)

add_executable(BulletinBoardConsistency
        article.cpp
        article.h
        clientside
        communicate.h
        communicate.x
        communicate_client.cpp
        communicate_clnt.cpp
        communicate_server.cpp
        communicate_svc.cpp
        communicate_xdr.cpp
        makefile
        peer.h
        README.md
        sequential_server.cpp
        sequential_server.h
        serverside
        startRPC)
