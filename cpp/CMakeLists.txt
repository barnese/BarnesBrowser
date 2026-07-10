cmake_minimum_required(VERSION 3.25)

project(BarnesBrowser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)

add_executable(main
    helpers/ssl_helper.cpp
    helpers/string_helpers.cpp
    html_tree/element.cpp
    html_tree/html_parser.cpp
    html_tree/text.cpp
    browser_scene.cpp
    browser_view.cpp
    layout.cpp
    main.cpp
    url.cpp
)

target_compile_options(main PRIVATE
    -Wall
    -Wextra
    -pedantic
)

target_link_libraries(main PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    Qt6::Widgets
    Qt6::Gui
    Qt6::Core
)