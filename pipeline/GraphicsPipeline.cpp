//
// Created by PentaKon on 8/11/2020.
//
#pragma warning(disable:4533)

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <iostream>
#include "GraphicsPipeline.h"
#include "Shaders.h"
#include "../buffers/Vertex.h"

/**
 * Takes shader code in a string and creates a VkShaderModule.
 *
 * @param device
 * @param shaderCode
 * @param error
 * @return
 */
VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &shaderCode, VkResult &error) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = shaderCode.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(shaderCode.data());
  error = VK_SUCCESS;

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    std::cerr << "Unable to create shader module" << std::endl;
    error = VK_ERROR_INVALID_SHADER_NV;
  }
  return shaderModule;
}

/**
 * Creates the graphics pipeline.
 * 1. Load shader codes from .spv files
 * 2. Create shader modules for each shader
 * 3. Create shader stages for each shader module
 * 4. Create vertex input state and input assembly state
 *
 * @param app
 * @return
 */
VkResult createGraphicsPipeline(Application &app) {
  std::vector<char> vertexShader{};
  readShaderFile("../shaders/vert.spv", vertexShader);

  std::vector<char> fragmentShader{};
  readShaderFile("../shaders/frag.spv", fragmentShader);

  VkResult errorCode = VK_SUCCESS;
  VkShaderModule vertShaderModule = createShaderModule(app.device, vertexShader, errorCode);
  VkShaderModule fragShaderModule = createShaderModule(app.device, fragmentShader, errorCode);

  if (errorCode != VK_SUCCESS) {
    goto throwError;
  }

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  // Describes the format of the vertex data that will be passed to the vertex shader
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  auto bindingDescription = getBindingDescription();
  auto attributeDescriptions = getAttributeDescriptions();
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

  // Describes the geometry that will be drawn (the vertices). Also describes if primitive restart should be enabled
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // Viewport is the portion of the framebuffer that will be rendered as output
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float) app.swapChainExtent.width;
  viewport.height = (float) app.swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // Any pixels outside the scissor rectangles will be discarded by the rasterizer
  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = app.swapChainExtent;

  // Create viewport state using the viewport and scissor rectangles
  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  // Configuration of the rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE; // if true clamps fragments outside of the near/far planes instead of discarding them
  rasterizer.rasterizerDiscardEnable = VK_FALSE; // disables the rasterizer. Use for when we just don't want to output fragments to framebuffer
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f; // line thickness is terms of number of fragments
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // specifies the type of face culling to use
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // specifies which faces are consider front faces
  rasterizer.depthBiasEnable = VK_FALSE; // might be used for shadow mapping
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  // Multisampling configuration
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f; // Optional
  multisampling.pSampleMask = nullptr; // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE; // Optional

  // Depth/stencil testing goes here

  // Color blending.
  // Combining the color the fragment shader returns with the color that is already in the framebuffer.
  // At the moment we disable all blending and simply pass the color of the fragment shader
  // directly as the new color of the framebuffer.

  // Color blending per framebuffer (we only have 1)
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE; // if blend -> false, the following is optional
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

  // Global color blending for all framebuffers. If logicOpEnable is true disables all per-framebuffer configurations apart from colorWriteMask
  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  // Possibly add pipeline dynamic state

  // Create pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0; // Optional
  pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

  errorCode = vkCreatePipelineLayout(app.device, &pipelineLayoutInfo, nullptr, &app.pipelineLayout);
  if (errorCode != VK_SUCCESS) {
    std::cerr << "Unable to create pipeline layout" << std::endl;
    goto throwError;
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = nullptr; // Optional
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = nullptr; // Optional
  pipelineInfo.layout = app.pipelineLayout;
  pipelineInfo.renderPass = app.renderPass;
  pipelineInfo.subpass = 0; // index of the supbass where the graphics pipeline will be used
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1; // Optional

  // the graphics pipeline creation is slow but can be drastically sped up by providing a pipeline cache
  // as the second argument which can even be serialized and stored for quick engine startup
  errorCode = vkCreateGraphicsPipelines(app.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &app.graphicsPipeline);
  if(errorCode != VK_SUCCESS) {
    std::cerr << "Unable to create graphics pipeline" << std::endl;
    goto throwError;
  }

  throwError:
  vkDestroyShaderModule(app.device, fragShaderModule, nullptr);
  vkDestroyShaderModule(app.device, vertShaderModule, nullptr);
  return errorCode;
}

/**
 * Create the render pass which will work on the framebuffer.
 * The framebuffer can have attachments for color, depth and stencil.
 * Each render pass can consist of multiple subpasses that apply
 * post-processing operations to the framebuffer, incrementally.
 * The subpasses must know what attachments the framebuffer that
 * they operate on has and the render pass must know which framebuffer
 * it will work on and what subpasses it will be performing.
 *
 * @param app
 * @return
 */
VkResult createRenderPass(Application &app) {
  // description of the attachments used in the framebuffer
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = app.imageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // attachment reference to be used by the render subpasses
  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // render subpass
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo  renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  VkResult errorCode = vkCreateRenderPass(app.device, &renderPassInfo, nullptr, &app.renderPass);
  throwOnError(errorCode, "Unable to create render pass");

  error:
  return errorCode;
}

/**
 * Creates the framebuffers that are the frontend to our swapchain images.
 * Assigns the created renderpass to each one of them.
 *
 * @param app
 * @return
 */
VkResult createFramebuffers(Application &app) {
  VkResult errorCode;
  app.swapChainFramebuffers.resize(app.swapChainImageViews.size());
  for (size_t i = 0; i < app.swapChainImageViews.size(); i++) {
    VkImageView attachments[] = {app.swapChainImageViews[i]};
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = app.renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = app.swapChainExtent.width;
    framebufferInfo.height = app.swapChainExtent.height;
    framebufferInfo.layers = 1;

    errorCode = vkCreateFramebuffer(app.device, &framebufferInfo, nullptr, &app.swapChainFramebuffers[i]);
    if(errorCode != VK_SUCCESS) {
      std::cerr << "Unable to create framebuffer" << std::endl;
      break;
    }
  }
  return errorCode;
}
