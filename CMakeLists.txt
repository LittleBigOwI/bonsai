cmake_minimum_required(VERSION 4.0)
project(bonsai)

set(CMAKE_CXX_STANDARD 17)

find_package(ftxui REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main ftxui::screen ftxui::dom ftxui::component)
