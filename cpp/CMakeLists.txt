cmake_minimum_required(VERSION 3.25)

project(BarnesBrowser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.11.0
)

FetchContent_MakeAvailable(Catch2)

find_package(OpenSSL REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)

add_executable(main
    commands/draw_rect.cpp
    commands/draw_text.cpp
    helpers/ssl_helper.cpp
    helpers/string_helpers.cpp
    html_tree/element.cpp
    html_tree/html_parser.cpp
    html_tree/text.cpp
    layout/block_layout.cpp
    layout/document_layout.cpp
    styles/css_parser.cpp
    browser_scene.cpp
    browser_view.cpp
    main.cpp
    url.cpp
)

target_include_directories(main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
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

add_executable(tests
    tests/string_helpers_tests.cpp
    helpers/string_helpers.cpp
)

target_include_directories(tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp
)

target_link_libraries(tests PRIVATE
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)

catch_discover_tests(tests)
