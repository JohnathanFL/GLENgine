project('glad', 'c')
gladLib = static_library('glad', ['glad.c', 'glad.h'])
inc = include_directories('.')
dep = declare_dependency(link_with: gladLib, include_directories: inc)
