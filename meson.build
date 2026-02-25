project(
  'Goobernatorial',
  'cpp',
  version: '0.0.1',
  meson_version: '>=1.8.0',
  default_options: ['cpp_std=c++20', 'b_ndebug=if-release', 'default_library=static'],
)

python = import('python').find_installation()

nanobind_dep = dependency('nanobind')
sdl_dep = dependency('sdl3')
deps = [
    nanobind_dep,
    sdl_dep,
]

includes = include_directories('include')
sources = [
    'src/goob_ext.cpp',
    'src/events.cpp',
    'src/renderer.cpp',
    'src/texture.cpp',
    'src/time.cpp',
    'src/window.cpp',
]

mod = python.extension_module(
  'goob',
  sources: sources,
  include_directories: includes,
  dependencies: deps,
  install: true,
)

stubgen = nanobind_dep.get_variable('stubgen')
custom_target(
  output: 'goob.pyi',
  depends: mod,
  command: [python, stubgen, '-m', 'goob', '-M', 'py.typed'],
  build_by_default: true,
)
