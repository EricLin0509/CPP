#pragma once

#include <vector>

namespace lve {

class MyPipeline {
    private:
        static std::vector<char> readShaderFile(const char* path);

        void createPipeline(const char* vertShaderPath, const char* fragShaderPath);
    public:
        MyPipeline(const char* vertShaderPath, const char* fragShaderPath);
        ~MyPipeline();
};

};