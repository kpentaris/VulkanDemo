//
// Created by PentaKon on 8/11/2020.
//

#include <vulkan/vulkan.h>
#include <iostream>
#include "../../Application.h"

/**
 * Creates the respective image views for the swapchain images.
 * The image view is akin to an SQL table view. It allows viewing
 * a portion of the image and dictates how to access it.
 *
 * @param app
 * @return
 */
VkResult createImageViews(Application &app) {
  app.swapChainImageViews.resize(app.swapChainImages.size());
  for (size_t i = 0; i < app.swapChainImages.size(); ++i) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = app.swapChainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = app.imageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // describes what the image's purpose is and which part of the image should be accessed
    // no mipmaps, no multi-layer images
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(app.device, &createInfo, nullptr, &app.swapChainImageViews[i]);
    if(result != VK_SUCCESS) {
      std::cerr << "Unable to create image views" << std::endl;
    }
    std::cout << "Created image views" << std::endl;
    return result;
  }
}
