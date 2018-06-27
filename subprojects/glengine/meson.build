project('glengine', 'cpp', default_options : ['cpp_std=c++17'])
compiler = meson.get_compiler('cpp')
enummap_script = find_program('subprojects/enummap/enummap.py')



entt = include_directories('subprojects/entt/src')

sdl2 = dependency('SDL2')
dl = compiler.find_library('dl')
vksdk_path = run_command('./getVulkanSDK.sh').stdout().strip()
vulkan_hppInc = include_directories(vksdk_path + '/include')
vulkanLib = compiler.find_library('vulkan')


includeDirs = vksdk_path + '/include ' + meson.current_source_dir() + '/subprojects/enummap'

enummap = generator(enummap_script,
                output: '@BASENAME@-enummap.cpp',
                arguments: ['@INPUT@', '@OUTPUT@', includeDirs])

enumMaps = enummap.process('Pipeline.hpp')
enumMapInc = include_directories('subprojects/enummap')

srcList = ['Geometry.cpp', 'Shader.cpp', 'Pipeline.cpp', 'Logger.cpp', 'Renderer.cpp', 'VulkanBackend.cpp']
glengineLib = static_library('glengine', [srcList, enumMaps],
    dependencies: [sdl2, dl, vulkanLib],
    include_directories: [vulkan_hppInc, entt, enumMapInc]
    )
glengineInc = include_directories('.')
glengine_dep = declare_dependency(link_with: [glengineLib], include_directories: [glengineInc, vulkan_hppInc], dependencies: vulkanLib)
