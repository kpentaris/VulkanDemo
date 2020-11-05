#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include "Physical.h"

uint32_t findQueueFamilies(VkPhysicalDevice device, bool &initSuccess) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  // we have found the available queue families, now we need to find which one supports VK__QUEUE_GRAPHICS_BIT
  int index = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      initSuccess = true;
      break;
    }
    ++index;
  }
  return index;
}

uint32_t isDeviceSuitable(VkPhysicalDevice device, bool &isSuitable) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  // we don't check features nor score devices at this time
  // we accept the dedicated GPU as a device
  if(deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    return 0;
  }
  bool queueFamilyFound = false;
  uint32_t queueFamilyIndex = findQueueFamilies(device, queueFamilyFound);
  isSuitable = queueFamilyFound;
  return queueFamilyIndex;
}

PhysicalDevice pickPhysicalDevice(VkInstance instance) {
  PhysicalDevice physicalDevice{};
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    std::cerr << "Failed to find GPUs with Vulkan support!" << std::endl;
    physicalDevice.foundDevice = false;
    return physicalDevice;
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  bool isSuitable = false;
  uint32_t deviceIndex = 0;
  for (const auto &device : devices) {
    deviceIndex = isDeviceSuitable(device, isSuitable);
    if (isSuitable) {
      physicalDevice.device = device;
      physicalDevice.index = deviceIndex;
      break;
    }
  }

  if (physicalDevice.device == VK_NULL_HANDLE) {
    std::cerr << "Failed to find a suitable GPU!" << std::endl;
    physicalDevice.foundDevice = false;
  } else {
    std::cout << "Found suitable dedicated GPU " << std::endl;
    physicalDevice.foundDevice = true;
  }
  return physicalDevice;
}
