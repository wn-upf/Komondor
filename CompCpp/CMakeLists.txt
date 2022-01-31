find_package(FLEX)
FLEX_TARGET(CompCppScanner lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

add_executable(cxx ${FLEX_CompCppScanner_OUTPUTS} token.cpp parser.cpp)
target_include_directories(cxx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(INSTALL_COMPCPP)
    install(TARGETS cxx DESTINATION bin)
endif()
