#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "validation/validation.h"
#include "devices/Devices.h"
#include "swapchain/Swapchain.h"
#include "swapchain/images/ImageViews.h"
#include "pipeline/GraphicsPipeline.h"
#include "pipeline/Commands.h"
#include <vector>
#include <iostream>

#define returnOnError(resultCode) if(resultCode != VK_SUCCESS) {return resultCode;}

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

Application app{};

void log_printSupportedExtensions(uint32_t glfwExtensionCount, const char **glfwExtensions) {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
  std::cout << "Supported Vulkan extensions:" << std::endl;
  for (const auto &extension : extensions) {
    std::cout << '\t' << extension.extensionName;
    const char *glfwExtension = *glfwExtensions;
    for (size_t count = 0; count < glfwExtensionCount; ++count) {
      if (strcmp(extension.extensionName, glfwExtension) == 0) {
        std::cout << " - GLFW" << std::endl;
        goto cnt;
      }
    }
    std::cout << std::endl;
    cnt:;
  }
}

std::vector<const char*> getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  log_printSupportedExtensions(glfwExtensionCount, glfwExtensions);

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

VkResult createInstance() {
  // Optional configuration struct. Provides some useful information to the
  // Vulkan driver (e.g. specific graphics engine).
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  auto extensions = getRequiredExtensions();
  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  // create info for debugging the VkInstance creation part
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo; // place here to ensure existence during the vkCreateInstance call

  if(enableValidationLayers) {
    if(!addValidationLayerSupport(createInfo, debugCreateInfo)) {
      return VK_ERROR_VALIDATION_FAILED_EXT;
    }
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  return vkCreateInstance(&createInfo, nullptr, &app.instance);
}

VkResult createSurface() {
  if(glfwCreateWindowSurface(app.instance, app.window, nullptr, &app.surface) != VK_SUCCESS) {
    std::cerr << "Failed to create window surface" << std::endl;
    return VK_ERROR_SURFACE_LOST_KHR;
  }
  return VK_SUCCESS;
}

VkResult createSemaphores(Application &app) {
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkResult errorCode;

  errorCode = vkCreateSemaphore(app.device, &semaphoreInfo, nullptr, &app.imageAvailableSemaphore);
  throwOnError(errorCode, "Unable to create image available semaphore")
  errorCode = vkCreateSemaphore(app.device, &semaphoreInfo, nullptr, &app.renderFinishedSemaphore);
  throwOnError(errorCode, "Unable to create render finished semaphore")

  error:
  return errorCode;
}

GLFWwindow *initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  return glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

VkResult initVulkan() {
  returnOnError(createInstance())

  if(enableValidationLayers) {
    setupDebugMessenger(app.instance, &app.debugMessenger);
  }
  returnOnError(createSurface())
  returnOnError(createDevice(app))
  returnOnError(createSwapChain(app))
  returnOnError(createImageViews(app))
  returnOnError(createRenderPass(app))
  returnOnError(createGraphicsPipeline(app))
  returnOnError(createFramebuffers(app))
  returnOnError(createCommandPool(app))
  returnOnError(createCommandBuffers(app))
  returnOnError(createSemaphores(app))
  return VK_SUCCESS;
}

void drawFrame() {

}

int mainLoop() {
  while (!glfwWindowShouldClose(app.window)) {
    glfwPollEvents();
    drawFrame();
  }
  return 0;
}

/**
 * Vulkan resource cleanup. The operation order MATTERS.
 *
 * @return
 */
int cleanup() {
  vkDestroySemaphore(app.device, app.renderFinishedSemaphore, nullptr);
  vkDestroySemaphore(app.device, app.imageAvailableSemaphore, nullptr);
  vkDestroyCommandPool(app.device, app.commandPool, nullptr);
  for (const auto &framebuffer : app.swapChainFramebuffers) {
    vkDestroyFramebuffer(app.device, framebuffer, nullptr);
  }
  vkDestroyPipeline(app.device, app.graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(app.device, app.pipelineLayout, nullptr);
  vkDestroyRenderPass(app.device, app.renderPass, nullptr);
  for (auto imageView : app.swapChainImageViews) {
    vkDestroyImageView(app.device, imageView, nullptr);
  }
  vkDestroySwapchainKHR(app.device, app.swapChain, nullptr);
  vkDestroyDevice(app.device, nullptr);

  if (enableValidationLayers) {
    cleanupDebugMessenger(app.instance, app.debugMessenger);
  }

  vkDestroySurfaceKHR(app.instance, app.surface, nullptr);
  vkDestroyInstance(app.instance, nullptr);
  glfwDestroyWindow(app.window);
  glfwTerminate();
  return 0;
}

int runApplication() {
  GLFWwindow *window = initWindow();
  app.window = window;

  int errCode;

  if ((errCode = initVulkan()) != VK_SUCCESS) {
    return errCode;
  }

  if ((errCode = mainLoop()) != VK_SUCCESS) {
    return errCode;
  }

  if ((errCode = cleanup()) != VK_SUCCESS) {
    return errCode;
  }

  return errCode;
}

int main() {
  return runApplication();
}
