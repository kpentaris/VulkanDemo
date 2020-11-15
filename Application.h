//
// Created by PentaKon on 7/11/2020.
//

#ifndef VULKANDEMO_APPLICATION_H
#define VULKANDEMO_APPLICATION_H

#define throwOnError(errorCode, message) if(errorCode != VK_SUCCESS) {std::cerr << message << std::endl; goto error;}

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
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat imageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass  renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    // Semaphores
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
} Application;
#endif //VULKANDEMO_APPLICATION_H
