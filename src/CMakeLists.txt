configure_file(list_of_macros.h ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

set(COST_HEADERS corsa_alloc.h
                 cost.h
                 priority_q.h)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/COST)
foreach(header ${COST_HEADERS})
        configure_file("../COST/${header}" ${CMAKE_CURRENT_BINARY_DIR}/COST/ COPYONLY)
endforeach()

add_subdirectory(learning_modules)
add_subdirectory(main)
add_subdirectory(methods)
add_subdirectory(structures)
