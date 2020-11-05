#ifndef VULKANDEMO_PHYSICAL_H
#define VULKANDEMO_PHYSICAL_H

typedef struct PhysicalDevice {
    VkPhysicalDevice device;
    uint32_t index;
    bool foundDevice;
} PhysicalDevice;

PhysicalDevice pickPhysicalDevice(VkInstance);

#endif //VULKANDEMO_PHYSICAL_H
