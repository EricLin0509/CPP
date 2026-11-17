#pragma once

#include "my_model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace lve {

struct TransformComponent {
    glm::vec3 translation{};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation;

    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    glm::mat4 mat4();

    glm::mat3 normalMatrix();
};

class GameObject {
    public:
        using id_t = unsigned int;

        static GameObject createGameObject() {
            static id_t current_id = 0;
            return GameObject(current_id++);
        }

        GameObject(const GameObject &) = delete;
        GameObject(GameObject &&) = default;
        GameObject &operator=(const GameObject &) = delete;
        GameObject &operator=(GameObject &&) = delete;

        id_t getID() { return id; }

        std::shared_ptr<MyModel> model{};
        glm::vec3 color{};
        TransformComponent transform{};
    private:
        id_t id;

        GameObject(id_t obj_id) : id(obj_id) {}

};

};