set(ML_NOM_HEADERS centralized_action_banning.h
                   multi_armed_bandits.h
                   rtot_algorithm.h)

foreach(header ${ML_NOM_HEADERS})
        configure_file("${header}" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach()
