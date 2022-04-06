project('mpc', 'c')

install_headers('mpc.h')

mpc_incdir = include_directories(['.'])

mpc = library(
  'mpc',
  'mpc.c',
  include_directories: mpc_incdir,
  install: true
)

pkg = import('pkgconfig')
pkg.generate(mpc)

mpc_dep = declare_dependency(
  include_directories: mpc_incdir,
  link_with: mpc,
)
