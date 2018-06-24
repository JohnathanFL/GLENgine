project('glengine', 'cpp', default_options : ['cpp_std=c++17'])
compiler = meson.get_compiler('cpp')


entt = include_directories('subprojects/entt/src')

sdl2 = dependency('SDL2')
dl = compiler.find_library('dl')
vksdk_path = run_command('./getVulkanSDK.sh').stdout().strip()
vulkan_hppInc = include_directories(vksdk_path + '/include')
vulkanLib = compiler.find_library('vulkan')

glengineLib = static_library('glengine',
    ['Geometry.cpp', 'Shader.cpp', 'Pipeline.cpp', 'Logger.cpp', 'Renderer.cpp', 'VulkanRendererBoilerplate.cpp'],
    dependencies: [sdl2, dl, vulkanLib],
    include_directories: [vulkan_hppInc, entt]
    )

glengineInc = include_directories('.')
glengine_dep = declare_dependency(link_with: [glengineLib], include_directories: [glengineInc, vulkan_hppInc], dependencies: vulkanLib)
