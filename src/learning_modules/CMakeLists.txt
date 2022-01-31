set(LM_HEADERS ml_model.h
               pre_processor.h)

foreach(header ${LM_HEADERS})
        configure_file("${header}" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach()

add_subdirectory(network_optimization_methods)
