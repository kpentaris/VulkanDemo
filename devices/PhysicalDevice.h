//
// Created by Ntinos on 7/11/2020.
//

#ifndef VULKANDEMO_PHYSICALDEVICE_H
#define VULKANDEMO_PHYSICALDEVICE_H

#include <vulkan/vulkan.h>

typedef struct PhysicalDevice {
    VkPhysicalDevice device;
    uint32_t graphicsQueueFamilyIdx;
    uint32_t presentationQueueFamilyIdx;
    bool foundDevice;
} PhysicalDevice;
#endif //VULKANDEMO_PHYSICALDEVICE_H
