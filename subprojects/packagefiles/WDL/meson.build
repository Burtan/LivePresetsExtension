project('WDL', ['c', 'cpp'], version: 'unknown',
        default_options: ['cpp_std=none'])

cc = meson.get_compiler('c')

# WDL
wdl_src = files(
        'WDL/wingui/wndsize.cpp'
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
        swell_dir = 'WDL' / 'swell'
        swell_public_args = ['-DSWELL_PROVIDED_BY_APP']
        if host_machine.system() == 'darwin'
                swell_sources = swell_dir / 'swell-modstub.mm'
                add_languages('objcpp')
                swell_dependencies += dependency('appleframeworks', modules: ['AppKit'])
        else
                swell_sources = swell_dir / 'swell-modstub-generic.cpp'
        endif

        swell = library('swell', swell_sources,
                        dependencies: swell_dependencies,
                        cpp_args: swell_public_args, objcpp_args: swell_public_args)
        swell_dep = declare_dependency(
                compile_args: swell_public_args,
                dependencies: [wdl_dep],
                link_with: swell)
        meson.override_dependency('SWELL', swell_dep)
endif
