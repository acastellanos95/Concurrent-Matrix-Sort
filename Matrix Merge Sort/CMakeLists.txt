cmake_minimum_required(VERSION 3.20)
project(Matrix_Merge_Sort)

set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
add_executable(Matrix_Merge_Sort main.cpp Utils.h)
target_link_libraries(Matrix_Merge_Sort PRIVATE Threads::Threads)