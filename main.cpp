#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "Errors.h"
#include "validation/validation.h"
#include "devices/Physical.h"
#include "devices/Logical.h"
#include <cstdlib>
#include <vector>
#include <iostream>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

typedef struct TriangleApplication {
    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    PhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
} TriangleApplication;

TriangleApplication app{};

void log_printSupportedExtensions(uint32_t glfwExtensionCount, const char **glfwExtensions) {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
  std::cout << "Supported Vulkan extensions:" << std::endl;
  for (const auto &extension : extensions) {
    std::cout << '\t' << extension.extensionName;
    const char *glfwExtension = *glfwExtensions;
    for (int count = 0; count < glfwExtensionCount; ++count) {
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

int createInstance() {
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
  createInfo.enabledExtensionCount = (uint32_t) extensions.size();
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

GLFWwindow *initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  return glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

int initVulkan() {
  createInstance();
  if(enableValidationLayers) {
    setupDebugMessenger(app.instance, &app.debugMessenger);
  }
  PhysicalDevice physicalDevice = pickPhysicalDevice(app.instance);
  if(physicalDevice.foundDevice) {
    app.physicalDevice = physicalDevice;
    const VkDeviceCreateInfo deviceCreateInfo = createLogicalDeviceInfo(physicalDevice.index);
    if(vkCreateDevice(physicalDevice.device, &deviceCreateInfo, nullptr, &app.device) != VK_SUCCESS) {
      std::cerr << "Failed to create logical device" << std::endl;
      return VK_ERROR_INITIALIZATION_FAILED;
    }
    vkGetDeviceQueue(app.device, physicalDevice.index, 0, &app.graphicsQueue);
  } else {
    return VK_ERROR_INITIALIZATION_FAILED;
  }
  return 0;
}

int mainLoop() {
  while (!glfwWindowShouldClose(app.window)) {
    glfwPollEvents();
  }
  return 0;
}

int cleanup() {
  if (enableValidationLayers) {
    cleanupDebugMessenger(app.instance, app.debugMessenger);
  }
  vkDestroyDevice(app.device, nullptr);
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
  if (runApplication() != NO_ERROR) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
