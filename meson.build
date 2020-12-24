project('akai', 'c')
gtkdep = dependency('gtk+-3.0')
configure_file(input: 'akai.ui', output: 'akai.ui', copy: true)
executable('akai', ['main.c', 'list.c'], dependencies : gtkdep)
