project('mcpp', 'cpp', default_options : ['cpp_std=c++17'])
compiler = meson.get_compiler('cpp')

sdl2 = dependency('SDL2')
GL = dependency('GL')
dl = compiler.find_library('dl')


glad = dependency('glad', fallback: ['glad', 'dep'])
subdir('glengine')

mcpp = executable('mcpp', ['src/main.cpp'], dependencies: glengine)
