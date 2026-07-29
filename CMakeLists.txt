cmake_minimum_required(VERSION 3.25)

project(BarnesBrowser LANGUAGES CXX C)

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

add_subdirectory(third_party/duktape)

add_executable(main
    src/commands/draw_line.cpp
    src/commands/draw_rect.cpp
    src/commands/draw_text.cpp
    src/helpers/ssl_helper.cpp
    src/helpers/string_helpers.cpp
    src/html_tree/element.cpp
    src/html_tree/html_parser.cpp
    src/html_tree/text.cpp
    src/layout/block_layout.cpp
    src/layout/document_layout.cpp
    src/layout/input_layout.cpp
    src/layout/line_layout.cpp
    src/layout/text_layout.cpp
    src/styles/css_parser.cpp
    src/styles/descendant_selector.cpp
    src/styles/style_helpers.cpp
    src/styles/tag_selector.cpp
    src/browser.cpp
    src/browser_tab.cpp
    src/browser_view.cpp
    src/chrome/chrome.cpp
    src/main.cpp
    src/url.cpp
)

# Enable automatic Meta-Object compiler support (used by Qt).
set_target_properties(main PROPERTIES 
    AUTOMOC ON
)

target_include_directories(main PRIVATE src)

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
    duktape
)

# Copy the assets folder into the build folder.
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

add_executable(tests
    src/tests/css_parser_tests.cpp
    src/tests/string_helpers_tests.cpp
    src/tests/url_tests.cpp

    src/helpers/ssl_helper.cpp
    src/helpers/string_helpers.cpp
    src/html_tree/element.cpp
    src/html_tree/html_parser.cpp
    src/html_tree/text.cpp
    src/styles/css_parser.cpp
    src/styles/descendant_selector.cpp
    src/styles/tag_selector.cpp
    src/url.cpp
)

target_include_directories(tests PRIVATE src)

target_link_libraries(tests PRIVATE
    Catch2::Catch2WithMain
    OpenSSL::SSL
    OpenSSL::Crypto
)

include(CTest)
include(Catch)

catch_discover_tests(tests)
