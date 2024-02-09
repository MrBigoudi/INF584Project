#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "model.hpp"

struct EntityTransform{
    glm::vec3 _Position = glm::vec3();
    glm::vec3 _Rotation = glm::vec3();
    glm::vec3 _Scale = glm::vec3(1.f, 1.f, 1.f);

    glm::mat4 getModel(){
        glm::mat4 identity = glm::mat4(1.f);

        glm::mat4 translationMatrix = glm::translate(
            identity, 
            _Position
        );

        glm::mat4 rotationX = glm::rotate(
            identity, 
            glm::radians(_Rotation.x), 
            glm::vec3(1, 0, 0)
        );
        glm::mat4 rotationY = glm::rotate(
            identity, 
            glm::radians(_Rotation.y), 
            glm::vec3(0, 1, 0)
        );
        glm::mat4 rotationZ = glm::rotate(
            identity, 
            glm::radians(_Rotation.z), 
            glm::vec3(0, 0, 1)
        );
        glm::mat4 rotationMatrix = rotationX * rotationY * rotationZ;

        glm::mat4 scaleMatrix = glm::scale(
            identity, 
            _Scale
        );

        glm::mat4 modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;
        return modelMatrix;
    }
};

struct EntityModel{
    ModelPtr _Model;
};