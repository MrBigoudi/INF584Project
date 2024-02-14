file(GLOB BE_SOURCE_FILES "*.cpp")

# create the library for the renderer of the engine
add_library(BigoudiEngine ${BE_SOURCE_FILES})

target_include_directories(BigoudiEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)

# set the cpp standards
target_link_libraries(BigoudiEngine PUBLIC cflags)

add_subdirectory(entityComponentSystem)
add_subdirectory(renderSubSystems)
add_subdirectory(inputs)
add_subdirectory(dep)