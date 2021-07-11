set(STRUCTURES_HEADERS action.h
                       controller_report.h
                       FIFO.h
                       logger.h
                       logical_nack.h
                       modulations.h
                       node_configuration.h
                       notification.h
                       performance.h
                       wlan.h)

foreach(header ${STRUCTURES_HEADERS})
        configure_file("${header}" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach()
