cmake_minimum_required(VERSION 3.17)
project(VulkanDemo)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_CXX_FLAGS /Wall)
#set(CMAKE_CXX_FLAGS_RELEASE /O2)

include_directories(
        ../Lib/glm
        ../Lib/glfw-3.3.2.bin.WIN64/include
        C:\\VulkanSDK\\1.2.154.1\\Include
)
link_directories(
        ../Lib/glfw-3.3.2.bin.WIN64/lib-vc2019
        C:\\VulkanSDK\\1.2.154.1\\Lib
)

link_libraries(vulkan-1.lib glfw3.lib)

add_executable(VulkanDemo
        main.cpp
#        test.cpp
        Errors.h
        validation/validation.cpp validation/validation.h
        devices/Devices.cpp devices/Devices.h swapchain/Swapchain.cpp swapchain/Swapchain.h Application.h devices/PhysicalDevice.h)
