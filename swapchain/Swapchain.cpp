//
// Created by PentaKon on 7/11/2020.
//

#include <vulkan/vulkan.h>
#include <vector>
#include <utility>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include "Swapchain.h"

/**
 * Selects the best swap surface format from the available ones for our swap chain.
 * If we can't find a BGRA 32-bit with SRGB color space we choose the first available format.
 *
 * @param availableFormats
 * @return
 */
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

/**
 * Attempts to select the presentation mode VK_PRESENT_MODE_MAILBOX_KHR if possible, otherwise
 * selects the VK_PRESENT_MODE_FIFO_KHR which is the only one that is guaranteed to be available
 * according to the spec.
 *
 * @param availablePresentModes
 * @return
 */
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

/**
 * Helper function that retrieves the available formats and presentation modes for the device-surface combo.
 *
 * @param device
 * @param surface
 * @return
 */
std::pair<std::vector<VkSurfaceFormatKHR>, std::vector<VkPresentModeKHR>>
getFormatsAndPresentationModes(VkPhysicalDevice device, VkSurfaceKHR surface) {
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
  }
  return std::pair{formats, presentModes};
}

/**
 * Check if the device and surface combination have swap chain support.
 *
 * @param device
 * @param surface
 * @return
 */
bool checkSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
  const std::pair<std::vector<VkSurfaceFormatKHR>, std::vector<VkPresentModeKHR>> &pair = getFormatsAndPresentationModes(device, surface);
  formats = pair.first;
  presentModes = pair.second;
  return !formats.empty() && !presentModes.empty();
}

/**
 * Creates the swapchain with our preferred format and presentation mode as well as
 * the VkExtent2D size that best matches the window.
 * 
 * @param app
 */
VkResult createSwapChain(Application &app) {
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(app.physicalDevice.device, app.surface, &capabilities);

  const std::pair<std::vector<VkSurfaceFormatKHR>, std::vector<VkPresentModeKHR>> &pair =
      getFormatsAndPresentationModes(app.physicalDevice.device, app.surface);

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(pair.first);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(pair.second);
  VkExtent2D extent = chooseSwapExtent(app.window, capabilities);

  // Minimum count of image handles we want to have in our swapchain.
  // Add one more than the minimum in case the driver does work and we
  // don't want to wait in order to acquire another image to render to.
  // Note that the Vulkan implementation is allowed to create more than that.
  uint32_t imageCount = capabilities.minImageCount + 1;
  // additionally, make sure to not exceed the maximum number of images. If maxImageCount == 0 it means there is no maximum
  if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
    imageCount = capabilities.maxImageCount;
  }

  // begin filling in the creation info struct
  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = app.surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1; // this is usually 1 except if we're doing stereoscopic 3D (e.g. VR?)
  // this means that we're rendering directly to the swapchain image. It is possible to render somewhere else, do post-processing and then transfer
  // that image into the swapchain image
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // we know that our graphics and presentation queue are the same so we explicitly use EXCLUSIVE sharing mode of queue images
  // if the queues were different we would be drawing into the graphics queue and submitting them to the presentation queue
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.queueFamilyIndexCount = 0; // optional, important in case the queue families are different
  createInfo.pQueueFamilyIndices = nullptr; // same as above

  createInfo.preTransform = capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE; // pointer to the old swapchain in case it changes. Swapchain changes happen on window resize

  VkResult result = vkCreateSwapchainKHR(app.device, &createInfo, nullptr, &app.swapChain);
  if(result != VK_SUCCESS) {
    std::cerr << "Unable to create swap chain" << std::endl;
    return result;
  }
  std::cout << "Successfully created swap chain" << std::endl;

  // retrieve swapchain image handles that have been created
  vkGetSwapchainImagesKHR(app.device, app.swapChain, &imageCount, nullptr);
  app.swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(app.device, app.swapChain, &imageCount, app.swapChainImages.data());

  app.imageFormat = createInfo.imageFormat;
  app.extent2D = createInfo.imageExtent;

  return VK_SUCCESS;
}
