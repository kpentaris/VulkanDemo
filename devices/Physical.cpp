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
}

bool isDeviceSuitable(VkPhysicalDevice
