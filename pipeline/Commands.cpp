//
// Created by PentaKon on 15/11/2020.
//

#include <iostream>
#include "Commands.h"
#include "../buffers/Vertex.h"

VkResult createCommandPool(Application &app) {
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = app.physicalDevice.graphicsQueueFamilyIdx;
  poolInfo.flags = 0; // optional

  VkResult errorCode = vkCreateCommandPool(app.device, &poolInfo, nullptr, &app.commandPool);
  if (errorCode != VK_SUCCESS) {
    std::cerr << "Unable to create command pool" << std::endl;
  }
  return errorCode;
}

/**
 * Creates a commend buffer for each defined swapchain framebuffer, attaches
 * the proper renderpass and framebuffer attachments, binds the graphics pipeline
 * and finally initiates the draw command.
 *
 * @param app
 * @return
 */
VkResult createCommandBuffers(Application &app) {
  app.commandBuffers.resize(app.swapChainFramebuffers.size());

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = app.commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t) app.commandBuffers.size();

  VkResult errorCode = vkAllocateCommandBuffers(app.device, &allocInfo, app.commandBuffers.data());
  throwOnError(errorCode, "Unable to allocate command buffers")

  for (size_t i = 0; i < app.commandBuffers.size(); ++i) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // optional
    beginInfo.pInheritanceInfo = nullptr; // optional

    errorCode = vkBeginCommandBuffer(app.commandBuffers[i], &beginInfo);
    if(errorCode != VK_SUCCESS) {
      break;
    }
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = app.renderPass;
    renderPassInfo.framebuffer = app.swapChainFramebuffers[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = app.swapChainExtent;

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(app.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(app.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, app.graphicsPipeline);

    VkBuffer vertexBuffers[] = {app.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(app.commandBuffers[i], 0, 1, vertexBuffers, offsets);

    vkCmdDraw(app.commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);

    vkCmdEndRenderPass(app.commandBuffers[i]);

    errorCode = vkEndCommandBuffer(app.commandBuffers[i]);
    throwOnError(errorCode, "Failed to end recording command buffer")
  }
  throwOnError(errorCode, "Failed to begin recording command buffer")
  error:
  return errorCode;
}
