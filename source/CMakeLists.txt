set(TARGET_NAME Tutorial4)

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
        usd
        sdf
        usdImaging
        usdImagingGL
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        qtadvanceddocking-qt6
)

target_compile_definitions(${TARGET_NAME}
    PRIVATE 
        NOMINMAX
        TBB_SUPPRESS_DEPRECATED_MESSAGES
)

target_compile_options(${TARGET_NAME} 
    PRIVATE
        /wd4996 # warning STL4029: std::atomic_*() overloads for shared_ptr are deprecated in C++20
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
add_subdirectory(model)
add_subdirectory(render)
add_subdirectory(resources)
add_subdirectory(camera)
add_subdirectory(ui)