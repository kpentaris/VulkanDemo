//
// Created by PentaKon on 7/11/2020.
//

#ifndef VULKANDEMO_SWAPCHAIN_H
#define VULKANDEMO_SWAPCHAIN_H

#include "../Application.h"

bool checkSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
VkResult createSwapChain(Application &app);
#endif //VULKANDEMO_SWAPCHAIN_H
