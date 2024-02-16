file(GLOB BE_DEP_SOURCE_FILES "*.cpp")

target_sources(BigoudiEngine PUBLIC ${BE_DEP_SOURCE_FILES})

target_include_directories(BigoudiEngine 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)

add_subdirectory(tinyObjectLoader)
add_subdirectory(stbImage)