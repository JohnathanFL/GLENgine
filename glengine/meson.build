glengineLib = static_library('glengine',
    ['Geometry.cpp', 'Input.cpp', 'Renderer.cpp', 'Shader.cpp'],
    dependencies: [sdl2, GL, dl, glad],
    )

glengineInc = include_directories('.')
glengine = declare_dependency(link_with: glengineLib, include_directories: glengineInc, dependencies: [glad])
