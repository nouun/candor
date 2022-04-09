project('candor', 'c')
candor_sources = [
  'libcandor/candor.c',
  'libcandor/cval.c',
  'libcandor/builtins.c',
  'libcandor/builtins/conditional.c',
  'libcandor/builtins/list.c',
  'libcandor/builtins/math.c',
]

mpc     = dependency('mpc')
libedit = dependency('libeditline')

incdirs = include_directories([
  'subprojects',
])

install_headers(
  'libcandor/candor.h',
  'libcandor/cval.h',
)

libcandor = library(
  'libcandor',
  candor_sources,
  include_directories: incdirs,
  dependencies: [
    mpc,
  ],
)

pkg = import('pkgconfig')
pkg.generate(libcandor)
  
libcandor_dep = declare_dependency(
  include_directories: incdirs,
  link_with: libcandor,
)

executable(
  'candor',
  'src/main.c',
  include_directories: incdirs,
  dependencies: [
    libcandor_dep,
    libedit,
    mpc,
  ],
)
