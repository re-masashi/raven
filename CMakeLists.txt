cmake_minimum_required(VERSION 3.11)
project(RavenGame)
set(CMAKE_CXX_STANDARD 17)
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
target_link_libraries(raven raylib)
