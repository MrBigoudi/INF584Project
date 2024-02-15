#include "errorHandler.hpp"

namespace BE{

void ErrorHandler::defaultCase(const std::string& msg, ErrorLevel level){
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
}

/**
    * Handle an error
    * @param error The error to handle
    * @param msg The error message to display
    * @param level The error level
*/
void ErrorHandler::handle(ErrorCode error, const std::string& msg, ErrorLevel level){
    switch(error){
        case NO_ERROR:
            break;
        case GLFW_ERROR:
            if(level == FATAL){
                glfwTerminate();
                defaultCase(msg, level);
            }
            break;
        default:
            defaultCase(msg, level);
            break;
    }
}

/**
    * Handle a vulkan error
    * @param error The error to handle
    * @param msg Th error message to display
    * @param level The error level
*/
void ErrorHandler::vulkanError(VkResult result, const std::string& msg, ErrorLevel level){
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
void ErrorHandler::glfwError(const std::string& msg, ErrorLevel level){
    handle(GLFW_ERROR, msg, level);
}


};