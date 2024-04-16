# resources
if host_machine.system() == 'windows'
    windows = import('windows')
    project_sources += windows.compile_resources('resource.rc',
                                                 include_directories: '.'
    )
endif