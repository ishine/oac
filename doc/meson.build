have_dot = find_program('dot', required: false).found()

doxyfile_conf = configuration_data()
doxyfile_conf.set('VERSION', oac_version)
doxyfile_conf.set('HAVE_DOT', have_dot)
doxyfile_conf.set('top_srcdir', top_srcdir)
doxyfile_conf.set('top_builddir', top_builddir)

doxyfile = configure_file(input: 'Doxyfile.in',
  output: 'Doxyfile',
  configuration: doxyfile_conf,
  install: false)

docdir = join_paths(get_option('datadir'), get_option('docdir'))

doc_inputs = [
  'customdoxygen.css',
  'footer.html',
  'header.html',
  'oac_logo.svg',
  top_srcdir + '/include/oac.h',
  top_srcdir + '/include/oac_multistream.h',
  top_srcdir + '/include/oac_defines.h',
  top_srcdir + '/include/oac_types.h',
  top_srcdir + '/include/oac_custom.h',
]

custom_target('doc',
  input: [ doxyfile ] + doc_inputs,
  output: [ 'html' ],
  command: [ doxygen, doxyfile ],
  install_dir: docdir,
  install: true)
