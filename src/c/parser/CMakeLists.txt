find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -std=c++11")

set(LEX_FLAGS "--nodefault")

set(YACC_FLAGS "-t --report=none")

#set_target_properties(lex.yy PROPERTIES COMPILE_FLAGS "-Wno-deprecated-register" )

# builds lex.yy.cpp
flex_target(lexer lex.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cpp
            COMPILE_FLAGS ${LEX_FLAGS}
)

# builds parser.tab.cpp and parser.tab.h
bison_target(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cpp
             COMPILE_FLAGS ${YACC_FLAGS}
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h
)

ADD_FLEX_BISON_DEPENDENCY(lexer parser)

set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libperfflow_parser.map")

set(perfflow_parser_sources
    perfflow_parser.cpp
)

set_source_files_properties(${FLEX_lexer_OUTPUTS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-register")

add_library(perfflow_parser SHARED
            ${perfflow_parser_sources}
            ${BISON_parser_OUTPUTS}
            ${FLEX_lexer_OUTPUTS}
)

target_include_directories(perfflow_parser PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
