//
// Created by Ntinos on 7/11/2020.
//

#include <vulkan/vulkan.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <algorithm>
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
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
  for (const auto& availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
        (uint32_t) width,
        (uint32_t) height
    };

    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
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

  return !formats.empty() && !presentModes.empty();
}
