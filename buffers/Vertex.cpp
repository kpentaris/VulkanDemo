//
// Created by PentaKon on 18/11/2020.
//

#include "Vertex.h"
#include "vulkan/vulkan.h"
#include "../Application.h"
#include <array>
#include <iostream>

/**
 * Creates the basic description of the Vertex object that will be used by the vertex buffer.
 * Essentially defines the stride and the fact that each stride represents a vertex not an instance of a vertex.
 *
 * @return
 */
VkVertexInputBindingDescription getBindingDescription() {
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(Vertex);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescription;
}

/**
 * Creates an array with descriptions for each attribute of the vertex.
 * The first attribute is the vertex position.
 * The second attribute is the vertex color.
 *
 * @return
 */
std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
  std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
  // vertex position attribute
  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(Vertex, pos);

  // vertex color attribute
  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Vertex, color);
  return attributeDescriptions;
}

/**
 * Based on the required memory properties, we find the matching GPU memory representation
 * that can satisfy the requirements. We query all available GPU memory types and select
 * not only the appropriate type but one that has properties we need such as the ability
 * to write directly to it from the CPU side.
 *
 * @param app
 * @param typeFilter
 * @param properties
 * @param errorCode
 * @return
 */
uint32_t findMemoryType(Application &app, uint32_t typeFilter, VkMemoryPropertyFlags properties, VkResult &errorCode) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(app.physicalDevice.device, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        memProperties.memoryTypes[i].propertyFlags & properties) {
      return i;
    }
  }
  std::cerr << "Unable to find suitable memory type" << std::endl;
  errorCode = VK_ERROR_MEMORY_MAP_FAILED;
  return -1;
}

/**
 * Creates a vertex buffer.
 * First decide how large the buffer will be based on the Vertex struct size and their amount.
 * Create the buffer.
 * Buffer creation does not allocate memory. We need to query the memory requirements for such a buffer.
 * Find a GPU memory type that matches our requirements.
 * Allocate memory from it.
 * Bind allocated memory to vertex buffer object.
 *
 * @param app
 * @return
 */
VkResult createVertexBuffer(Application &app) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = sizeof(vertices[0]) * vertices.size();
  bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VkResult errorCode = vkCreateBuffer(app.device, &bufferInfo, nullptr, &app.vertexBuffer);
  throwOnError(errorCode, "Unable to create vertex buffer")
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(app.device, app.vertexBuffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(app, memRequirements.memoryTypeBits,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, errorCode);
  errorCode = vkAllocateMemory(app.device, &allocInfo, nullptr, &app.vertexBufferMemory);
  throwOnError(errorCode, "Unable to allocate vertex buffer memory")

  // the offset is used if the buffer memory is larger than the vertex buffer size
  // and the beginning of the vertex buffer is not at the start (offset 0).
  // If the offset is non-zero, then it is required to be divisible by memRequirements.alignment.
  errorCode = vkBindBufferMemory(app.device, app.vertexBuffer, app.vertexBufferMemory, 0);
  throwOnError(errorCode, "Unable to bind buffer memory to vertex buffer")

  void *data;
  vkMapMemory(app.device, app.vertexBufferMemory, 0, bufferInfo.size, 0, &data);
  memcpy(data, vertices.data(), (size_t) bufferInfo.size);
  // it is possible that the copied memory is not directly visible to the buffer side
  // this is solved by using GPU memory with the HOST_COHERENT property
  vkUnmapMemory(app.device, app.vertexBufferMemory);
  // the Vulkan spec declares that the copied memory will be visible to the GPU as of the ntext call to vkQueueSubmit

  error:
  return errorCode;
}
