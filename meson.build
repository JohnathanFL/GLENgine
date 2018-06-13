project('mcpp', 'cpp', default_options : ['cpp_std=c++17'])
compiler = meson.get_compiler('cpp')

glengine = dependency('glengine', fallback: ['glengine', 'glengine_dep'])
mcpp = executable('mcpp', ['src/main.cpp'], dependencies: glengine)
