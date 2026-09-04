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

set(CMAKE_DISABLE_FIND_PACKAGE_WrapVulkanHeaders ON)
find_package(Qt6 REQUIRED COMPONENTS 
    Core
    Gui 
    OpenGLWidgets
    Widgets
)

add_subdirectory(third_party/duktape)

# Add the old "dukglue" library, we need to circumvent its
# old CMake version requirements, hence the SOURCE_SUBDIR hack.
# We only need the header files so it can be added as an interface.
# Note: SYSTEM forces the compiler to ignore warnings from this library.
FetchContent_Declare(
    dukglue_src
    GIT_REPOSITORY https://github.com/Aloshi/dukglue.git
    GIT_TAG master
    SOURCE_SUBDIR cmake-do-not-add
)
FetchContent_MakeAvailable(dukglue_src)
add_library(dukglue INTERFACE)
target_include_directories(dukglue SYSTEM INTERFACE ${dukglue_src_SOURCE_DIR}/include)

add_executable(main
    src/accessibility/accessibility_node.cpp
    src/chrome/chrome.cpp
    src/commands/blend.cpp
    src/commands/composited_layer.cpp
    src/commands/draw_composited_layer.cpp
    src/commands/draw_line.cpp
    src/commands/draw_outline.cpp
    src/commands/draw_rounded_rect.cpp
    src/commands/draw_text.cpp
    src/commands/transform.cpp
    src/helpers/measure_time.cpp
    src/helpers/ssl_helper.cpp
    src/helpers/string_helpers.cpp
    src/helpers/thread_registry.cpp
    src/html_tree/element.cpp
    src/html_tree/html_parser.cpp
    src/html_tree/node_helpers.cpp
    src/html_tree/text.cpp
    src/layout/block_layout.cpp
    src/layout/document_layout.cpp
    src/layout/input_layout.cpp
    src/layout/layout_effects_helpers.cpp
    src/layout/layout_helpers.cpp
    src/layout/line_layout.cpp
    src/layout/text_layout.cpp
    src/styles/css_parser.cpp
    src/styles/descendant_selector.cpp
    src/styles/numeric_animation.cpp
    src/styles/pseudo_class_selector.cpp
    src/styles/style_helpers.cpp
    src/styles/tag_selector.cpp
    src/scripting/js_context.cpp
    src/tasks/task_runner.cpp
    src/browser.cpp
    src/browser_tab.cpp
    src/browser_view.cpp
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

# Debug Options - Enable in tasks.json.
option(
    SHOW_COMPOSITED_LAYER_BORDERS 
    "Enables composition layer borders" 
    OFF
)

if(SHOW_COMPOSITED_LAYER_BORDERS)
    target_compile_definitions(main PRIVATE 
        SHOW_COMPOSITED_LAYER_BORDERS=$<BOOL:${SHOW_COMPOSITED_LAYER_BORDERS}>
    )
endif()

target_link_libraries(main PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGLWidgets
    Qt6::Widgets
    duktape
    dukglue
)

# Copy the assets folder into the build folder.
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

add_executable(tests
    src/tests/test_css_parser.cpp
    src/tests/test_string_helpers.cpp
    src/tests/test_url.cpp

    src/helpers/ssl_helper.cpp
    src/helpers/string_helpers.cpp
    src/html_tree/element.cpp
    src/html_tree/html_parser.cpp
    src/html_tree/text.cpp
    src/styles/css_parser.cpp
    src/styles/descendant_selector.cpp
    src/styles/pseudo_class_selector.cpp
    src/styles/tag_selector.cpp
    src/url.cpp
)

target_include_directories(tests PRIVATE src)

target_link_libraries(tests PRIVATE
    Catch2::Catch2WithMain
    OpenSSL::SSL
    OpenSSL::Crypto
    Qt6::Core
    Qt6::Gui
)

include(CTest)
include(Catch)

catch_discover_tests(tests)
