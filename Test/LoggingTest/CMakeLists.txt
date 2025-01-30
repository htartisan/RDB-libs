# CMakeList.txt : CMake project for LoggingTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("LoggingTest")

# Add source to this project's executable.
add_executable (LoggingTest "LoggingTest.cpp" "LoggingTest.h")

include_directories
    (
        ../../Libs/spdlog/include
        ../../Src
    )
	
