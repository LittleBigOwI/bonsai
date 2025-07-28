cmake_minimum_required(VERSION 3.14)
project(bonsai)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ftxui REQUIRED)

add_executable(bonsai
    main.cpp
    ui/sidebar.cpp
)

target_link_libraries(bonsai
    ftxui::screen
    ftxui::dom
    ftxui::component
)