#pragma once

#include "my_engine_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace lve {

class MyDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(VulkanDevice &device) : device{device} {}

    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<MyDescriptorSetLayout> build() const;

   private:
    VulkanDevice &device;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };

  MyDescriptorSetLayout(
      VulkanDevice &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~MyDescriptorSetLayout();
  MyDescriptorSetLayout(const MyDescriptorSetLayout &) = delete;
  MyDescriptorSetLayout &operator=(const MyDescriptorSetLayout &) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

 private:
  VulkanDevice &device;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

  friend class MyDescriptorWriter;
};

class MyDescriptorPool {
 public:
  class Builder {
   public:
    Builder(VulkanDevice &device) : device{device} {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<MyDescriptorPool> build() const;

   private:
    VulkanDevice &device;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };

  MyDescriptorPool(
      VulkanDevice &device,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~MyDescriptorPool();
  MyDescriptorPool(const MyDescriptorPool &) = delete;
  MyDescriptorPool &operator=(const MyDescriptorPool &) = delete;

  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

  void resetPool();

 private:
  VulkanDevice &device;
  VkDescriptorPool descriptorPool;

  friend class MyDescriptorWriter;
};

class MyDescriptorWriter {
 public:
  MyDescriptorWriter(MyDescriptorSetLayout &setLayout, MyDescriptorPool &pool);

  MyDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  MyDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

 private:
  MyDescriptorSetLayout &setLayout;
  MyDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};

}  // namespace lve