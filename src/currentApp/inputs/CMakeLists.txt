file(GLOB INPUTS_SOURCE_FILES "*.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${INPUTS_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)