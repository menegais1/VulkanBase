cmake_minimum_required(VERSION 3.17)
project(VulkanBase)

set(CMAKE_CXX_STANDARD 14)
find_package(Vulkan REQUIRED)
ADD_SUBDIRECTORY(Dependencies/glfw-3.3.2)
add_executable(VulkanBase src/main.cpp
        src/VulkanSetup.cpp
        src/VulkanSetup.h
        src/VulkanStructures.h
        src/VulkanDebug.h
        src/FileManagers/FileLoader.h
        src/FileManagers/FileLoader.cpp
        src/FileManagers/Bitmap/Bitmap.h
        src/FileManagers/Bitmap/Bitmap.cpp
        src/VulkanHelpers.h src/CommandBufferUtils.cpp src/CommandBufferUtils.h)
target_link_libraries(VulkanBase glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)
