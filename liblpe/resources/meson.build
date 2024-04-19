# resources
if host_machine.system() == 'windows'
    windows = import('windows')
    project_sources += windows.compile_resources('resource.rc',
                                                 include_directories: '.'
    )
elif host_machine.system() != 'windows'
    project_sources += custom_target('res',
                                     command: ['php', '../subprojects/WDL/WDL/swell/swell_resgen.php', '../liblpe/resources/resource.rc'],
                                     build_always: true,
                                     output: ['resource.rc_mac_dlg', 'resource.rc_mac_menu']
    )
endif