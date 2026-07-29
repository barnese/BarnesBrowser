add_library(duktape STATIC
    duktape.c
    duktape.h
    duk_config.h
)

target_include_directories(duktape PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
