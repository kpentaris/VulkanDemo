//
// Created by PentaKon on 8/11/2020.
//

#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include "GraphicsPipeline.h"
#include "Shaders.h"

VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &shaderCode, VkResult &error) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = shaderCode.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
  error = VK_SUCCESS;

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    std::cerr << "Unable to create shader module" << std::endl;
    error = VK_ERROR_INVALID_SHADER_NV;
  }
  return shaderModule;
}

VkResult createGraphicsPipeline(Application &app) {
  std::vector<char> vertexShader{};
  readShaderFile("../shaders/vert.spv", vertexShader);

  std::vector<char> fragmentShader{};
  readShaderFile("../shaders/frag.spv", fragmentShader);
  
  VkResult error = VK_SUCCESS;
  VkShaderModule vertShaderModule = createShaderModule(app.device, vertexShader, error);
  VkShaderModule fragShaderModule = createShaderModule(app.device, fragmentShader, error);

  if(error != VK_SUCCESS) {
    return error;
  }

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  vkDestroyShaderModule(app.device, fragShaderModule, nullptr);
  vkDestroyShaderModule(app.device, vertShaderModule, nullptr);
  return error;
}
