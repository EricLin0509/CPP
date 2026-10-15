#pragma once

#include "my_engine_device.hpp"

#include <vector>

namespace lve {

struct PipelineConfigInfo {
};

class MyPipeline {
    private:
        VulkanDevice& device;
        VkPipeline pipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;

        static std::vector<char> readShaderFile(const char* path);

        void createPipeline(const char* vertShaderPath, const char* fragShaderPath, const PipelineConfigInfo& configInfo);

        void createShaderModule(const std::vector<char> bin_code, VkShaderModule *shaderModule);
    public:
        MyPipeline(
            VulkanDevice& device,
            const PipelineConfigInfo& configInfo,
            const char* vertShaderPath,
            const char* fragShaderPath);
        ~MyPipeline();

        MyPipeline(const MyPipeline&) = delete;
        void operator=(const MyPipeline&) = delete;

        static PipelineConfigInfo defaultConfigInfo(uint32_t width, uint32_t height);
};

};