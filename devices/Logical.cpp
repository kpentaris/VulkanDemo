//
// Created by Ntinos on 5/11/2020.
//

#include <vulkan/vulkan.h>
#include "../validation/validation.h"

VkDeviceCreateInfo createLogicalDeviceInfo(uint32_t physicalDeviceIndex) {
  VkDeviceQueueCreateInfo queueCreateInfo{};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = physicalDeviceIndex;
  queueCreateInfo.queueCount = 1;
  float queuePriority = 1.0f;
  queueCreateInfo.pQueuePriorities = &queuePriority;

  VkPhysicalDeviceFeatures deviceFeatures{}; // leave empty for now

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = &queueCreateInfo;
  createInfo.queueCreateInfoCount = 1;
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = 0;
  if(enableValidationLayers) {
    addValidationLayerSupport(createInfo);
  } else {
    createInfo.enabledLayerCount = 0;
  }
  return createInfo;
}
