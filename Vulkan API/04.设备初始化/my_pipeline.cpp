#include "my_pipeline.hpp"

#include <iostream>
#include <fstream>

using namespace lve;

MyPipeline::MyPipeline(const char* vertShaderPath, const char* fragShaderPath)
{
    createPipeline(vertShaderPath, fragShaderPath);
}

MyPipeline::~MyPipeline(){}


std::vector<char> MyPipeline::readShaderFile(const char* path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open shader file");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

void MyPipeline::createPipeline(const char* vertShaderPath, const char* fragShaderPath)
{
    /* 读取着色器文件 */
    auto vertCode = readShaderFile(vertShaderPath);
    auto fragCode = readShaderFile(fragShaderPath);

    std::cout << "Vertex shader code size: " << vertCode.size() << " " << fragCode.size() << "\n";
    std::cout << "Fragment shader code size: " << fragCode.size() << "\n";
}