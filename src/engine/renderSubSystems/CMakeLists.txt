file(GLOB RENDER_SUB_SYSTEMS_SOURCE_FILES "*.cpp")

target_sources(BigoudiEngine PUBLIC ${RENDER_SUB_SYSTEMS_SOURCE_FILES})

target_include_directories(BigoudiEngine 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)