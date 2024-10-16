cmake_minimum_required(VERSION 3.29)
project(SSHConnectTools C)

set(CMAKE_C_STANDARD 11)

add_executable(SSHConnectTools main.c
        cJSON.c)
