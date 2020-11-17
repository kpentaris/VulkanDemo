//
// Created by PentaKon on 7/11/2020.
//

#ifndef VULKANDEMO_APPLICATION_H
#define VULKANDEMO_APPLICATION_H

#define returnOnError(errorCode) if(errorCode != VK_SUCCESS) {return errorCode;}
#define throwOnError(errorCode, message) if(errorCode != VK_SUCCESS) {std::cerr << message << std::endl; goto error;}

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include "devices/PhysicalDevice.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

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

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    // Semaphores
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    bool framebufferResized = false; // manual handling of window resize event
} Application;
#endif //VULKANDEMO_APPLICATION_H
