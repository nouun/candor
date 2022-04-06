(use-modules
  (guix packages))

(use-package-modules
  bdw-gc
  build-tools
  commencement
  gdb
  llvm
  libedit
  ninja
  pkg-config
  valgrind)

(packages->manifest
  (list
    gcc-toolchain
    gdb
    valgrind

    meson
    ninja
    pkg-config

    editline
    libgc))

