#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>

#include "model.hpp"

struct EntityTransform{
    glm::vec3 _Position = glm::vec3();
    glm::vec3 _Rotation = glm::vec3();
    glm::vec3 _Scale = glm::vec3(1.f, 1.f, 1.f);

    glm::mat4 getModel(){
        const float c3 = glm::cos(_Rotation.z);
        const float s3 = glm::sin(_Rotation.z);
        const float c2 = glm::cos(_Rotation.x);
        const float s2 = glm::sin(_Rotation.x);
        const float c1 = glm::cos(_Rotation.y);
        const float s1 = glm::sin(_Rotation.y);
        return glm::mat4{
            {
                _Scale.x * (c1 * c3 + s1 * s2 * s3),
                _Scale.x * (c2 * s3),
                _Scale.x * (c1 * s2 * s3 - c3 * s1),
                0.0f,
            },
            {
                _Scale.y * (c3 * s1 * s2 - c1 * s3),
                _Scale.y * (c2 * c3),
                _Scale.y * (c1 * c3 * s2 + s1 * s3),
                0.0f,
            },
            {
                _Scale.z * (c2 * s1),
                _Scale.z * (-s2),
                _Scale.z * (c1 * c2),
                0.0f,
            },
            {_Position.x, _Position.y, _Position.z, 1.0f}
        };
    }
};

struct EntityModel{
    ModelPtr _Model = nullptr;
};