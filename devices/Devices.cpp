#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <iostream>
#include "../Application.h"
#include "../validation/validation.h"
#include "../swapchain/Swapchain.h"

/**
 * The list of phys device extension names that are mandatory
 */
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

/**
 * For the given physical device, find the queue family index that supports VK_QUEUE_GRAPHICS_BIT
 * as well as presentation for the given surface.
 *
 * @param surface
 * @param device
 * @param initSuccess
 * @return
 */
uint32_t findQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice device, bool &initSuccess) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  // we have found the available queue families, now we need to find which one supports VK__QUEUE_GRAPHICS_BIT
  int index = 0;
  for (const auto &queueFamily : queueFamilies) {
    VkBool32 presentationSupport = false;
    // we decide to select a device that supports graphics and presentation
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentationSupport);
      if (presentationSupport) {
        initSuccess = true;
        break;
      }
    }
    ++index;
  }
  return index;
}

/**
 * Check if the physical device supports the mandatory {@link deviceExtensions}.
 *
 * @param device
 * @return
 */
bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  // nice hack for ticking off all required extensions
  // copy the required extensions list and if found, remove it
  // we should end with an empty list of required extensions if all were found
  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

/**
 * Checks if the physical device is suitable for our surface.
 *
 * @param surface
 * @param device
 * @param isSuitable
 * @return
 */
uint32_t isDeviceSuitable(VkSurfaceKHR surface, VkPhysicalDevice device, bool &isSuitable) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  // we don't check features nor score devices at this time
  // we accept the dedicated GPU as a device
  if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    return 0;
  }
  bool queueFamilyFound = false;
  uint32_t queueFamilyIndex = findQueueFamilies(surface, device, queueFamilyFound);
  isSuitable = queueFamilyFound && checkDeviceExtensionSupport(device) && checkSwapChainSupport(device, surface);
  if (isSuitable) {
    std::cout << "Device " << deviceProperties.deviceName << " is suitable" << std::endl;
  } else {
    std::cout << "Device " << deviceProperties.deviceName << " is not suitable" << std::endl;
  }
  return queueFamilyIndex;
}

/**
 * Selects one of the available physical devices based on our surface.
 *
 * @param instance
 * @param surface
 * @return
 */
PhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
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
    deviceIndex = isDeviceSuitable(surface, device, isSuitable);
    if (isSuitable) {
      physicalDevice.device = device;
      physicalDevice.graphicsQueueFamilyIdx = deviceIndex;
      physicalDevice.presentationQueueFamilyIdx = deviceIndex;
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

/**
 * Creates the VkDevice for our application.
 *
 * @param instance
 * @param pVkDevice
 * @param pPhysicalDevice
 * @param surface
 * @return
 */
VkResult createDevice(Application &app) {
  const PhysicalDevice physicalDevice = pickPhysicalDevice(app.instance, app.surface);
  if (!physicalDevice.foundDevice) {
    std::cerr << "Unable to find suitable physical device" << std::endl;
    return VK_ERROR_INITIALIZATION_FAILED;
  }
  app.physicalDevice = physicalDevice;

  VkDeviceQueueCreateInfo queueCreateInfo{}; // dies
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = physicalDevice.graphicsQueueFamilyIdx;
  queueCreateInfo.queueCount = 1;

  float queuePriority = 1.0f; // dies
  queueCreateInfo.pQueuePriorities = &queuePriority;

  VkPhysicalDeviceFeatures deviceFeatures{}; // leave empty for now // dies

  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
  deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
  if (enableValidationLayers) {
    addValidationLayerSupport(deviceCreateInfo);
  } else {
    deviceCreateInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(physicalDevice.device, &deviceCreateInfo, nullptr, &app.device) != VK_SUCCESS) {
    std::cerr << "Failed to create logical device" << std::endl;
    return VK_ERROR_INITIALIZATION_FAILED;
  }
  vkGetDeviceQueue(app.device, app.physicalDevice.graphicsQueueFamilyIdx, 0, &app.graphicsQueue);
  vkGetDeviceQueue(app.device, app.physicalDevice.presentationQueueFamilyIdx, 0, &app.presentQueue);
  return VK_SUCCESS;
}
