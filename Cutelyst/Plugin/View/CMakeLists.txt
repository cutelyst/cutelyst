find_package(Grantlee)

if (Grantlee_FOUND)
    message(STATUS "Grantlee templating found.")
    add_subdirectory(Grantlee)
elseif(Grantlee_FOUND)
    message(STATUS "Grantlee not found.")
endif (Grantlee_FOUND)
