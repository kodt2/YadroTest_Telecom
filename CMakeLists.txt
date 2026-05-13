cmake_minimum_required(VERSION 3.16)

project(YadroTestTelecom LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(dungeon_simulator
    Bot.cpp
    Cave.cpp
)

target_include_directories(dungeon_simulator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(dungeon_simulator PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(dungeon_simulator PRIVATE /W4 /permissive-)
else()
    target_compile_options(dungeon_simulator PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(yadro_telecom main.cpp)
target_link_libraries(yadro_telecom PRIVATE dungeon_simulator)