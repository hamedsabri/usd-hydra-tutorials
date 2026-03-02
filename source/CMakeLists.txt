set(TARGET_NAME Tutorial1)

add_executable(${TARGET_NAME})

# -----------------------------------------------------------------------------
# sources
# -----------------------------------------------------------------------------
target_sources(${TARGET_NAME}
    PRIVATE
        main.cpp
)

# -----------------------------------------------------------------------------
# include directories
# -----------------------------------------------------------------------------
target_include_directories(${TARGET_NAME} 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# -----------------------------------------------------------------------------
# link libraries
# -----------------------------------------------------------------------------
target_link_libraries(${TARGET_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        qtadvanceddocking-qt6
)

target_compile_features(${TARGET_NAME} 
    PRIVATE
        cxx_std_20
)

# -----------------------------------------------------------------------------
# install
# -----------------------------------------------------------------------------
install(TARGETS ${TARGET_NAME}
    LIBRARY
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    install(FILES $<TARGET_PDB_FILE:${TARGET_NAME}> 
            DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL
    )
endif()

#------------------------------------------------------------------------------
# directories
#------------------------------------------------------------------------------
add_subdirectory(resources)
add_subdirectory(ui)