#include <vulkan/vulkan.h>
#include <vector>

bool pickPhysicalDevice(VkInstance instance) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if(deviceCount == 0) {
    return VK_ERROR_INITIALIZATION_FAILED;
  }
  std::vector<VkPhysicalDevice> devices(deviceCount); 
  vkEnumeratePhysicalDevices(instance, $deviceCount, devices.data());

  for (const auto& device : devices) {
    if (isDeviceSuitable(device)) {
      physicalDevice = device;
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    std::cout << "Failed to find a suitable GPU!" << std::endl;
    return VK_ERROR_INITIALIZATION_FAILED;
  }
}

bool isDeviceSuitable(VkPhysicalDevice device) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
  return true;
}
