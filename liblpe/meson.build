# version information

#version_cc = vcs_tag(
#    input : configure_file(
#        input : 'version.cc.in',
#        output : 'version.cc.in',
#        configuration : version_data,
#    ),
#    output : 'version.cc',
#)

project_sources += files(
    'LivePresetsExtension.cpp',
    'ExtensionEntry.cpp'
)

subdir('controller')
subdir('data')
subdir('ui')
subdir('util')
subdir('resources')