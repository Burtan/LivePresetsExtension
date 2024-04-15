project('WDL', ['c', 'cpp'],
        version: 'unknown',
        default_options: ['cpp_std=none']
)

cc = meson.get_compiler('c')

# WDL
wdl_src = files(
        'WDL/wingui/wndsize.cpp',
#        'WDL/wingui/virtwnd-slider.cpp',
#        'WDL/wingui/virtwnd-listbox.cpp',
#        'WDL/wingui/virtwnd-iconbutton.cpp',
#        'WDL/wingui/virtwnd-iaccessible.cpp',
#        'WDL/wingui/virtwnd.cpp',
)
wdl_dep = declare_dependency(
        compile_args: '-DWDL_NO_DEFINE_MINMAX',
        include_directories: ['WDL'],
        sources: wdl_src
)
meson.override_dependency('WDL', wdl_dep)

# SWELL
if host_machine.system() != 'windows'
        swell_dependencies = [wdl_dep]
        swell_public_args = ['-DSWELL_PROVIDED_BY_APP']
        if host_machine.system() == 'darwin'
                swell_sources = swell_dir / 'swell-modstub.mm'
                add_languages('objcpp')
                swell_dependencies += dependency('appleframeworks', modules: ['AppKit'])
        else
                swell_sources = files(
#                        'WDL/swell/test.cpp',
#                        'WDL/swell/swell-wnd-generic.cpp',
                        'WDL/swell/swell-modstub-generic.cpp',
#                        'WDL/swell/swell-miscdlg-generic.cpp',
#                        'WDL/swell/swell-misc-generic.cpp',
#                        'WDL/swell/swell-menu-generic.cpp',
#                        'WDL/swell/swell-kb-generic.cpp',
#                        'WDL/swell/swell-ini.cpp',
#                        'WDL/swell/swell-generic-headless.cpp',
#                        'WDL/swell/swell-generic-gdk.cpp',
#                        'WDL/swell/swell-gdi-lice.cpp',
#                        'WDL/swell/swell-gdi-generic.cpp',
#                        'WDL/swell/swell-dlg-generic.cpp',
#                        'WDL/swell/swell-appstub-generic.cpp',
#                        'WDL/swell/swell.cpp'
                )
        endif

        swell = static_library('swell', swell_sources,
                        #dependencies: swell_dependencies,
                        cpp_args: swell_public_args,
                        objcpp_args: swell_public_args
        )
        swell_dep = declare_dependency(
                compile_args: swell_public_args,
                dependencies: [wdl_dep],
                link_whole: [swell]
        )
        meson.override_dependency('SWELL', swell_dep)
endif
