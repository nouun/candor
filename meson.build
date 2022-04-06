project('candor', 'c')
candor_sources = [
  'src/main.c',
  'src/cval.c',
  'src/builtins.c',
  'src/builtins/conditional.c',
  'src/builtins/list.c',
  'src/builtins/math.c',
  'src/builtins/sugar.c',
]

mpc     = dependency('mpc')
libedit = dependency('libeditline')

incdirs = include_directories([
  'subprojects',
])

candor_lib = library(
  'candor',
  candor_sources,
  dependencies : [
    libedit,
    mpc,
  ],
  include_directories: incdirs,
)

  
executable(
  'candor',
  candor_sources,
  dependencies : [
    libedit,
    mpc,
  ],
  include_directories: incdirs,
)
