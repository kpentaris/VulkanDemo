//
// Created by PentaKon on 7/11/2020.
//

#ifndef VULKANDEMO_APPLICATION_H
#define VULKANDEMO_APPLICATION_H

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include "devices/PhysicalDevice.h"

typedef struct Application {
    GLFWwindow *window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    PhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentationQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat imageFormat;
    VkExtent2D extent2D;

    std::vector<VkImageView> swapChainImageViews;
} Application;
#endif //VULKANDEMO_APPLICATION_H
