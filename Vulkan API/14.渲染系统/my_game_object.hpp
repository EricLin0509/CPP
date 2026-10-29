#pragma once

#include "my_model.hpp"

#include <memory>

namespace lve {

struct Transform2dComponent {
    glm::vec2 translation{};
    glm::vec2 scale{1.f, 1.f};
    float rotation;

    glm::mat2 mat2() {
        const float sin = glm::sin(rotation);
        const float cos = glm::cos(rotation);
        glm::mat2 rotMatrix = {{cos, sin}, {-sin, cos}};

        glm::mat2 scaleMat{{scale.x, 0.f}, {0.f, scale.y}};
        return rotMatrix * scaleMat;
    }
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
        Transform2dComponent transform2d{};
    private:
        id_t id;

        GameObject(id_t obj_id) : id(obj_id) {}

};

};