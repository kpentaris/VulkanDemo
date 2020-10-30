//
// Created by Ntinos on 30/10/2020.
//

#ifndef VULKANDEMO_VALIDATION_H
#define VULKANDEMO_VALIDATION_H

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &);

bool addValidationLayerSupport(VkInstanceCreateInfo&, VkDebugUtilsMessengerCreateInfoEXT&);

bool setupDebugMessenger(VkInstance, VkDebugUtilsMessengerEXT *);

void cleanupDebugMessenger(VkInstance, VkDebugUtilsMessengerEXT);

#endif //VULKANDEMO_VALIDATION_H
