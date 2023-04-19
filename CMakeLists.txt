cmake_minimum_required(VERSION 3.24)
project(Checkersv)

set(CMAKE_CXX_STANDARD 17)

add_executable(Checkersv main.cpp pawn.hpp)

#add_executable(CheckersCplusplus main.cpp pawn.cpp board.cpp game.cpp)

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR C:/SFML/lib/cmake/SFML)

find_package(SFML COMPONENTS system window graphics audio network REQUIRED)

include_directories(C:/SFML/include)

target_link_libraries(Checkersv sfml-system sfml-window sfml-graphics sfml-audio sfml-network)

