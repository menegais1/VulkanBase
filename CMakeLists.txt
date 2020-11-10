cmake_minimum_required(VERSION 3.17)
project(VulkanBase)

set(CMAKE_CXX_STANDARD 14)
find_package(Vulkan REQUIRED)
ADD_SUBDIRECTORY(Dependencies/glfw-3.3.2)
add_executable(VulkanBase main.cpp)
target_link_libraries(VulkanBase glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)
