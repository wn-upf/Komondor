set(METHODS_HEADERS agent_methods.h
                    auxiliary_methods.h
                    backoff_methods.h
                    modulations_methods.h
                    notification_methods.h
                    output_generation_methods.h
                    power_channel_methods.h
                    spatial_reuse_methods.h
                    time_methods.h)

foreach(header ${METHODS_HEADERS})
    configure_file("${header}" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach()
