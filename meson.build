project('akai', 'c')
gtkdep = dependency('gtk+-3.0')
glibdep = dependency('glib-2.0')
jsondep = dependency('json-glib-1.0')
configure_file(input: 'akai.ui', output: 'akai.ui', copy: true)
executable('akai', ['main.c', 'list.c'], dependencies : gtkdep)

executable('server', ['server.c', 'list.c'], dependencies : [glibdep, jsondep])
