file(GLOB ECS_SOURCE_FILES "*.cpp")

target_sources(${PROJECT_NAME} PUBLIC ${ECS_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)

add_subdirectory(systems)