//
// Created by PentaKon on 18/11/2020.
//

#ifndef VULKANDEMO_VERTEX_H
#define VULKANDEMO_VERTEX_H

#include "glm/glm.hpp"
#include <vector>
#include <array>
#include "vulkan/vulkan.h"
#include "../Application.h"

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

VkVertexInputBindingDescription getBindingDescription();
std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
VkResult createVertexBuffer(Application &app);

#endif //VULKANDEMO_VERTEX_H
