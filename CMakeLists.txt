cmake_minimum_required(VERSION 3.14)
project(cppexample)

set(CMAKE_CXX_STANDARD 14)


#------------------------------------------------------------------------------
#  Dependencies.
#------------------------------------------------------------------------------

include(cmake/FindRocksDb.cmake)

add_executable(rocksdb-main rocksdb-flatbuffer-test-main.cpp)
target_link_libraries(rocksdb-main rocksdb)