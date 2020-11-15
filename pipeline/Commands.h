//
// Created by PentaKon on 15/11/2020.
//

#ifndef VULKANDEMO_COMMANDS_H
#define VULKANDEMO_COMMANDS_H

#include <vulkan/vulkan.h>
#include "../Application.h"

VkResult createCommandPool(Application&);
VkResult createCommandBuffers(Application&);
#endif //VULKANDEMO_COMMANDS_H
