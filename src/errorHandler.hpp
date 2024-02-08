#pragma once

#include <cstdlib>
#include <iostream>

#include <stdarg.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

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
    IO_ERROR,
    NOT_INITIALIZED_ERROR,
    UNKNOWN_VALUE_ERROR,
    BAD_VALUE_ERROR,
    OUT_OF_RANGE_ERROR,
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

    public:
        /**
         * Handle an error
         * @param error The error to handle
         * @param msg The error message to display
         * @param level The error level
        */
        static void handle(ErrorCode error, const std::string& msg = "", ErrorLevel level = FATAL){
            switch(error){
                case NO_ERROR:
                    break;
                case GLFW_ERROR:
                    if(level == FATAL)
                        glfwTerminate();
                    // continue to default
                default:
                    switch(level){
                        case FATAL:
                            fprintf(stderr, "%s", msg.c_str());
                            fprintf(stderr, "Exiting the program!\n");
                            exit(EXIT_FAILURE);
                            break;
                        case WARNING:
                            fprintf(stderr, "%s", msg.c_str());
                            fprintf(stderr, "Warning, continue the program!\n");
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }

        /**
         * Handle a vulkan error
         * @param error The error to handle
         * @param msg Th error message to display
         * @param level The error level
        */
        static void vulkanError(VkResult result, const std::string& msg = "", ErrorLevel level = FATAL){
            if(result == VK_SUCCESS){
                return;
            }
            handle(VULKAN_ERROR, msg, level);
        }

        /**
         * Handle a glfw error
         * @param msg Th error message to display
         * @param level The error level
        */
        static void glfwError(const std::string& msg, ErrorLevel level = FATAL){
            handle(GLFW_ERROR, msg, level);
        }

};