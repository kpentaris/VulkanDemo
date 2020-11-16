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

std::vector<const char *> getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  log_printSupportedExtensions(glfwExtensionCount, glfwExtensions);

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

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

  if (enableValidationLayers) {
    if (!addValidationLayerSupport(createInfo, debugCreateInfo)) {
      return VK_ERROR_VALIDATION_FAILED_EXT;
    }
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  return vkCreateInstance(&createInfo, nullptr, &app.instance);
}

VkResult createSurface() {
  if (glfwCreateWindowSurface(app.instance, app.window, nullptr, &app.surface) != VK_SUCCESS) {
    std::cerr << "Failed to create window surface" << std::endl;
    return VK_ERROR_SURFACE_LOST_KHR;
  }
  return VK_SUCCESS;
}

VkResult createSyncObjects(Application &app) {
  app.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  app.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  app.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  app.imagesInFlight.resize(app.swapChainImages.size(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkResult errorCode;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    errorCode = vkCreateSemaphore(app.device, &semaphoreInfo, nullptr, &app.imageAvailableSemaphores[i]);
    throwOnError(errorCode, "Unable to create image available semaphore")
    errorCode = vkCreateSemaphore(app.device, &semaphoreInfo, nullptr, &app.renderFinishedSemaphores[i]);
    throwOnError(errorCode, "Unable to create render finished semaphore")
    errorCode = vkCreateFence(app.device, &fenceInfo, nullptr, &app.inFlightFences[i]);
    throwOnError(errorCode, "Unable to create fence")
  }

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
  VkResult errorCode = createInstance();
  returnOnError(errorCode)

  if (enableValidationLayers) {
    setupDebugMessenger(app.instance, &app.debugMessenger);
  }
  errorCode = createSurface();
  returnOnError(errorCode)
  errorCode = createDevice(app);
  returnOnError(errorCode)
  errorCode = createSwapChain(app);
  returnOnError(errorCode)
  errorCode = createImageViews(app);
  returnOnError(errorCode)
  errorCode = createRenderPass(app);
  returnOnError(errorCode)
  errorCode = createGraphicsPipeline(app);
  returnOnError(errorCode)
  errorCode = createFramebuffers(app);
  returnOnError(errorCode)
  errorCode = createCommandPool(app);
  returnOnError(errorCode)
  errorCode = createCommandBuffers(app);
  returnOnError(errorCode)
  errorCode = createSyncObjects(app);
  returnOnError(errorCode)
  return VK_SUCCESS;
}

VkResult drawFrame() {
  vkWaitForFences(app.device, 1, &app.inFlightFences[app.currentFrame], VK_TRUE, UINT64_MAX);
  uint32_t imageIndex; // refers to the index of the acquired swap chain image from the swapChainImages. We use that index to pick the correct command buffer

  // vkAcquire does not seem to guarantee that it will provide a swapchain image that is not in use. We have to manually synchronize on the images
  // as well using the inFlightFences (which are used for synchronizing all resources for each frame, guaranteeing they are used only on one frame
  // at a time)
  VkResult errorCode = vkAcquireNextImageKHR(app.device, app.swapChain, UINT64_MAX, app.imageAvailableSemaphores[app.currentFrame], VK_NULL_HANDLE, &imageIndex);
  throwOnError(errorCode, "Failed to acquire next image")
  // it is possible that we've been assigned an images from the swapchain that is still 'in-flight' and we must wait for it to become available
  if(app.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(app.device, 1, &app.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
  }
  app.imagesInFlight[imageIndex] = app.inFlightFences[app.currentFrame];

  VkSubmitInfo submitInfo{}; // command buffer submission info
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  // the waitSemaphores array and waitStages array are matched 1-1. The Xth indexed semaphore will be used at the Xth indexed stage
  VkSemaphore waitSemaphores[] = {app.imageAvailableSemaphores[app.currentFrame]};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // does this mean the semaphore will be signaled once we exit the frag-shader?
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &app.commandBuffers[imageIndex];

  // specify which semaphores to signal once the command buffer has finished execution
  VkSemaphore signalSemaphores[] = {app.renderFinishedSemaphores[app.currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(app.device, 1, &app.inFlightFences[app.currentFrame]);
// both a signal semaphore (render finished) and fences are used for synchronization on the queue operations
  errorCode = vkQueueSubmit(app.graphicsQueue, 1, &submitInfo, app.inFlightFences[app.currentFrame]);
  throwOnError(errorCode, "Failed to submit draw command buffer")

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;
  VkSwapchainKHR swapChains[] = {app.swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr; // optional

  errorCode = vkQueuePresentKHR(app.presentQueue, &presentInfo);
  throwOnError(errorCode, "Queue present failed")

  app.currentFrame = (app.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; // advance to next frame
  error:
  return errorCode;
}

int mainLoop() {
  VkResult errorCode = VK_SUCCESS;
  while (!glfwWindowShouldClose(app.window)) {
    glfwPollEvents();
    errorCode = drawFrame();
    if (errorCode != VK_SUCCESS) {
      break;
    }
  }
  vkDeviceWaitIdle(app.device);
  vkQueueWaitIdle(app.presentQueue);
  return errorCode;
}

/**
 * Vulkan resource cleanup. The operation order MATTERS.
 *
 * @return
 */
int cleanup() {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    vkDestroySemaphore(app.device, app.renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(app.device, app.imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(app.device, app.inFlightFences[i], nullptr);
  }
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

  int errorCode = initVulkan();
  returnOnError(errorCode)
  errorCode = mainLoop();
  returnOnError(errorCode)
  errorCode = cleanup();
  returnOnError(errorCode)

  return errorCode;
}

int main() {
  return runApplication();
}
