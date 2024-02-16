#pragma once

#include <cstdlib>
#include <iostream>

#include <stdarg.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

namespace BE{


/**
 * The error level to handle different cases
*/
enum ErrorLevel{
    FATAL,
    WARNING,
};

/**
 * The abstract error codes
*/
enum ErrorCode{
    NO_ERROR, 

    GLFW_ERROR,
    VULKAN_ERROR,
    TINYOBJ_ERROR,
    STB_IMAGE_ERROR,

    IO_ERROR,
    NOT_INITIALIZED_ERROR,
    UNKNOWN_VALUE_ERROR,
    BAD_VALUE_ERROR,
    OUT_OF_RANGE_ERROR,
    UNEXPECTED_VALUE_ERROR,

    SYSTEM_ERROR,
};

/**
 * The class to handle errors
*/
class ErrorHandler{
    private:
        /**
         * Private constructor to make the class purely virtual
        */
        ErrorHandler(){};

    private:
        static void defaultCase(const std::string& msg = "", ErrorLevel level = FATAL);

    public:
        /**
         * Handle an error
         * @param error The error to handle
         * @param msg The error message to display
         * @param level The error level
        */
        static void handle(ErrorCode error, const std::string& msg = "", ErrorLevel level = FATAL);

        /**
         * Handle a vulkan error
         * @param error The error to handle
         * @param msg Th error message to display
         * @param level The error level
        */
        static void vulkanError(VkResult result, const std::string& msg = "", ErrorLevel level = FATAL);

        /**
         * Handle a glfw error
         * @param msg Th error message to display
         * @param level The error level
        */
        static void glfwError(const std::string& msg, ErrorLevel level = FATAL);

};

};