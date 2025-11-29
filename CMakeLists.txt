cmake_minimum_required(VERSION 3.20)
project(RavenGame)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always -Wall -Wextra -Wpedantic -Wconversion -Wshadow")

find_package(raylib REQUIRED)

add_executable(raven 
 src/core/main.cpp 
 src/game/game.cpp 
 src/game/generateChunk.cpp 
 src/game/player.cpp
 src/game/frustumCulling.cpp
 src/game/structures.cpp
 src/game/sky.cpp
 src/game/vegetation.cpp
 src/game/water.cpp
)

target_link_libraries(raven PRIVATE raylib)
target_include_directories(raven PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
