project_sources += files(
    'LivePresetsExtension.cpp',
    'ExtensionEntry.cpp'
)

subdir('controller')
subdir('data')
subdir('ui')
subdir('util')

# TODO must adapt the gendialog command
#dialog = custom_target('dialog.rc', command: [gendialog], capture: true, output: 'dialog.rc')
#if host_machine.system() == 'windows'
#    win32_resources += windows.compile_resources(dialog)
#else
#    project_sources += dialog
#endif
