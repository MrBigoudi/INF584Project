file(GLOB DEP_STB_IMAGE_SOURCE_FILES "*.cpp")

target_sources(BigoudiEngine PUBLIC ${DEP_STB_IMAGE_SOURCE_FILES})

target_include_directories(BigoudiEngine 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)