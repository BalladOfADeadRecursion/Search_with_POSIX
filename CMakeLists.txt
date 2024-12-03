cmake_minimum_required(VERSION 3.28)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

# Добавляем опцию компилятора -pthread
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_MAKE_PROGRAM "make -j4")
set(SOURCE_FILES
        #[[source1.txt
        source2.txt
        source3.txt
        source4.txt
        source5.txt
        source6.txt
        source7.txt
        source8.txt
        source9.txt
        source10.txt]]
        test.txt
)

add_executable(untitled main.cpp)
include_directories(/home/ballad/CLionProjects/untitled)
target_link_libraries(untitled pthread)