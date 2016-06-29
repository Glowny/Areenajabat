#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace arena
{

    template <typename T>
    inline T lerp(T v0, T v1, T t) {
        return (1 - t)*v0 + t*v1;
    }

    // transform value by matrix
    inline void transform(const glm::vec2& value, const glm::mat4& matrix, glm::vec2* result)
    {
        result->x = (value.x * matrix[0].x) + (value.y * matrix[1].x) + matrix[3].x;
        result->y = (value.x * matrix[0].y) + (value.y * matrix[1].y) + matrix[3].y;
    }

    inline void transform(const glm::vec2& value, const glm::quat& rotation, glm::vec2* result)
    {
        glm::vec3 rot1(rotation.x + rotation.x, rotation.y + rotation.y, rotation.z + rotation.z);
        glm::vec3 rot2(rotation.x, rotation.x, rotation.w);
        glm::vec3 rot3(1.f, rotation.y, rotation.z);
        glm::vec3 rot4(rot1*rot2);
        glm::vec3 rot5(rot1*rot3);


        result->x = (float)((double)value.x * (1.0 - (double)rot5.y - (double)rot5.z) + (double)value.y * ((double)rot4.y - (double)rot4.z));
        result->y = (float)((double)value.x * ((double)rot4.y + (double)rot4.z) + (double)value.y * (1.0 - (double)rot4.x - (double)rot5.z));
    }

    inline void decompose(const glm::mat4& mtx, glm::vec2* position, glm::vec2* scale, float* rotation)
    {
        glm::vec3 pos3(glm::uninitialize), scale3(glm::uninitialize), skew3(glm::uninitialize);
        glm::vec4 pers(glm::uninitialize);
        glm::quat rotQ(glm::uninitialize);

        glm::decompose(mtx, scale3, rotQ, pos3, skew3, pers);

        glm::vec2 direction(glm::uninitialize);
        transform(glm::vec2(1.f, 0.f), rotQ, &direction);

        *rotation = -glm::atan(direction.y, direction.x);
        *position = glm::vec2(pos3.x, pos3.y);
        *scale = glm::vec2(scale3.x, scale3.y);
    }
}