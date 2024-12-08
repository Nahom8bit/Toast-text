cmake_minimum_required(VERSION 3.16)
project(IconGenerator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(generate_icons generate_icons.cpp)

target_link_libraries(generate_icons PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
) 