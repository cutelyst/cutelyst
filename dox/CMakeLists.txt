if(UNIX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cutelyst-wsgi.1.in
        ${CMAKE_CURRENT_BINARY_DIR}/cutelyst-wsgi2.1
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cutelyst-wsgi2.1 DESTINATION ${MANDIR}/man1)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cutelyst-wsgi.1.in
        ${CMAKE_CURRENT_BINARY_DIR}/cutelyst2.1
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cutelyst2.1 DESTINATION ${MANDIR}/man1)
endif()
