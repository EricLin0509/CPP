#pragma once

#include "my_engine_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace lve {

class MyModel {
    public:

        struct Vertex {
            glm::vec2 position;
            glm::vec3 color;
            
            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        MyModel(VulkanDevice& device, const std::vector<Vertex>& vertices);
        ~MyModel();

        MyModel(const MyModel&) = delete;
        MyModel &operator=(const MyModel&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        VulkanDevice& device;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;

        void createVertexBuffer(const std::vector<Vertex>& vertices);
};

};