set(MAIN_HEADERS agent.h
                 central_controller.h
                 node.h
                 traffic_generator.h)

foreach(header ${MAIN_HEADERS})
        configure_file("${header}" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach()

configure_file(komondor_main.cc ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

set(CXX_OUT ${CMAKE_CURRENT_BINARY_DIR}/komondor_main.cxx
            ${CMAKE_CURRENT_BINARY_DIR}/compcxx_komondor_main.h)
add_custom_command(
    OUTPUT ${CXX_OUT}
    COMMAND cxx komondor_main.cc
    MAIN_DEPENDENCY komondor_main.cc
    DEPENDS cxx
)

add_executable(komondor komondor_main.cxx)
install(TARGETS komondor DESTINATION bin)
