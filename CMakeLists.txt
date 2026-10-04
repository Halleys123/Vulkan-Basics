cmake_minimum_required(VERSION 4.3)

project(Vulkan)

find_package(Vulkan REQUIRED)

set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")

add_subdirectory(externals/GLFW)
add_subdirectory(externals/glm)

add_executable(main ./main.cpp ./src/VulkanRenderer.cpp)
target_include_directories(main PRIVATE ./include)

if(MSVC)
    target_link_options(main PRIVATE "/SUBSYSTEM:CONSOLE")
      target_compile_options(main PRIVATE /EHsc)
endif()

if(CONFIG==Debug)
  set(DEBUG_MODE 1)
endif()

target_compile_definitions(main PRIVATE
    GLFW_INCLUDE_VULKAN
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    VK_USE_PLATFORM_WIN32_KHR # Although it can be used, try not to use this
    $<$<CONFIG:Debug>:DEBUG_MODE>
)

target_link_libraries(main PRIVATE glfw)
target_link_libraries(main PRIVATE glm::glm)

target_link_libraries(main PRIVATE Vulkan::Vulkan)

# install(TARGETS main RUNTIME DESTINATION bin)
# set(CPACK_PACKAGE_NAME "VulkanApp")
# set(CPACK_PACKAGE_VERSION "1.0.0")
# set(CPACK_GENERATOR "ZIP") # Can be ZIP, NSIS (installer), TGZ, etc.
# include(CPack)
