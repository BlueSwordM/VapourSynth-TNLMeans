project('TNLMeans', 'cpp',
  default_options: ['buildtype=release', 'warning_level=2', 'b_lto=true', 'b_ndebug=if-release', 'cpp_std=c++11'],
  meson_version: '>=0.51.0'
)

cxx = meson.get_compiler('cpp')

gcc_syntax = cxx.get_argument_syntax() == 'gcc'

if gcc_syntax
  vapoursynth_dep = dependency('vapoursynth', version: '>=55').partial_dependency(compile_args: true, includes: true)
  install_dir = vapoursynth_dep.get_variable(pkgconfig: 'libdir') / 'vapoursynth'
else
  vapoursynth_dep = []
  install_dir = get_option('libdir') / 'vapoursynth'
endif

config_h = declare_dependency(
    sources: vcs_tag(
        command: ['git', 'describe', '--tags', '--long', '--always'],
        input: 'config.h.in',
        output: 'config.h'
    )
)

sources = [
  'AlignedMemory.cpp',
  'TNLMeans.cpp',
  'Plugin.cpp',
]

shared_module('tnlmeans', sources,
  dependencies: [vapoursynth_dep, config_h],
  install: true,
  install_dir: install_dir,
  gnu_symbol_visibility: 'hidden'
)
