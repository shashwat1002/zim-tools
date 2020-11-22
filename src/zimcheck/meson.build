compiler = meson.get_compiler('cpp')
if compiler.get_id() == 'gcc' and host_machine.system() == 'linux'
  # C++ std::thread is implemented using pthread on linux by gcc
  thread_dep = dependency('threads')
else
  thread_dep = dependency('', required:false)
endif

executable('zimcheck', 'main.cpp', 'checks.cpp', '../tools.cpp',
  dependencies: [libzim_dep, thread_dep],
  install: true)


