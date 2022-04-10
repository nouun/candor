project('candor', 'c')


stdlib = get_option('stdlib')
if stdlib == ''
  stdlib = join_paths(
    get_option('prefix'),
    get_option('datadir'),
    'candor', 'stdlib'
  )
endif

config = configuration_data()
config.set('version', '0.1.1')
config.set('stdlib', stdlib)
configure_file(
  input: 'config.h.in',
  output: 'config.h',
  configuration: config
)


##################
#   candor lib   #
##################

candor_sources = [
  'candor/candor.c',
  'candor/cval.c',
  'candor/cenv.c',
  'candor/parser.c',
  'candor/builtins.c',
  'candor/builtins/conditional.c',
  'candor/builtins/list.c',
  'candor/builtins/math.c',
]

mpc     = dependency('mpc')
libedit = dependency('libeditline')

incdirs = include_directories([
  'subprojects',
  '.'
])

install_headers('candor.h')
install_subdir('stdlib', install_dir: stdlib, strip_directory: true)

candor_lib = library(
  'candor',
  candor_sources,
  include_directories: incdirs,
  dependencies: [
    mpc,
  ],
  install: true,
)

pkg = import('pkgconfig')
pkg.generate(candor_lib)

candor_dep = declare_dependency(
  include_directories: incdirs,
  link_with: candor_lib,
)


##################
#     candor     #
##################

executable(
  'candor',
  'src/main.c',
  include_directories: incdirs,
  dependencies: [
    candor_dep,
    libedit,
    mpc,
  ],
)
