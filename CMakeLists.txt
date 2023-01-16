cmake_minimum_required(VERSION 3.24)
project(fixObfuscatedFilenames C)

set(CMAKE_C_STANDARD 11)

add_executable( fixObfuscatedFilenames
                fixObfuscatedFilenames.c)

install( TARGETS fixObfuscatedFilenames
         RUNTIME DESTINATION /usr/bin )
