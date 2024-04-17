# resources
if host_machine.system() == 'windows'
    windows = import('windows')
    project_sources += windows.compile_resources('resource.rc',
                                                 include_directories: '.'
    )
elif host_machine.system() != 'windows'
    run_command('php', '../../subprojects/WDL/WDL/swell/swell_resgen.php', 'resource.rc', check: true)

#    project_sources += custom_target('version.h',
#                                     command: [genres],
#                                     capture: true,
#                                     output: 'version.h'
#    )

    # TODO use rc generator for swell

#//setWorkingDir("$projectDir/WDL-library/main/cpp/WDL/swell/")
#executable = "php"
#val scriptPath = "$projectDir/WDL-library/src/main/cpp/WDL/swell/mac_resgen.php"
#val rcPath = "$projectDir/src/main/headers/resources/resource.rc"
#args(scriptPath, rcPath)
#
#doLast {
#ant.withGroovyBuilder {
#val srcPath = "$projectDir/src/main/headers/resources/resource.rc_mac_"
#"move"("file" to "${srcPath}dlg", "todir" to "${buildDir}/res")
#"move"("file" to "${srcPath}menu", "todir" to "${buildDir}/res")
#}
#}
endif