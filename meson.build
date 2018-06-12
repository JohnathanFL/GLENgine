project('mcpp', 'cpp', default_options : ['cpp_std=c++17'])
compiler = meson.get_compiler('cpp')

sdl2 = dependency('SDL2')
dl = compiler.find_library('dl')

subdir('glengine')

mcpp = executable('mcpp', ['src/main.cpp'], dependencies: glengine)
