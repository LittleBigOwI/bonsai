cmake_minimum_required(VERSION 3.14)
project(bonsai)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ftxui REQUIRED)

add_executable(bonsai
    src/main.cpp
    
    src/core/scanner.cpp
    
    # src/interface/sidebar.cpp
    # src/interface/piechart.cpp
    
    src/utils/colors.cpp
    src/utils/dataformat.cpp
    
    src/widgets/canvas_ext.cpp
)

target_link_libraries(bonsai
    ftxui::screen
    ftxui::dom
    ftxui::component
)