
compiler = meson.get_compiler('cpp')

vulkan_hppInc = include_directories(get_option('VulkanSDK_Path') + '/include')
vulkanLib = compiler.find_library('vulkan')

glengineLib = static_library('glengine',
    ['Geometry.cpp', 'Input.cpp', 'Renderer.cpp', 'Shader.cpp', 'Logger.cpp', 'ResourceCache.cpp'],
    dependencies: [sdl2, dl, vulkanLib],
    include_directories: vulkan_hppInc
    )

glengineInc = include_directories('.')
glengine = declare_dependency(link_with: [glengineLib], include_directories: [glengineInc, vulkan_hppInc], dependencies: vulkanLib)
