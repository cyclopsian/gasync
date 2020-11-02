# SPDX-FileCopyrightText: 2020 Jason Francis <jason@cycles.network>
# SPDX-License-Identifier: CC0-1.0

project(
  'gasync',
  'c',
  version: '0.1',
  meson_version: '>=0.50.0',
  license: 'CC0-1.0',
  default_options: [
    'c_std=gnu11',
    'warning_level=2'
  ],
)

gio = dependency('gio-2.0')

gasync = library('gasync', 'gasync.c', dependencies: [gio],
                 install: true, install_dir: get_option('libdir'))
install_headers('gasync.h')
gasync_dep = declare_dependency(link_with: [gasync],
                                dependencies: [gio],
                                include_directories: '.')
executable('gasync-example', 'gasync-example.c', dependencies: [gasync_dep])
