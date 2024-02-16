file(GLOB BE_CORE_SOURCE_FILES "*.cpp")

target_sources(BigoudiEngine PUBLIC ${BE_CORE_SOURCE_FILES})

target_include_directories(BigoudiEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)

add_subdirectory(utils)
add_subdirectory(vulkan)
add_subdirectory(entityComponentSystem)
add_subdirectory(gameplay)
add_subdirectory(renderSubSystems)
