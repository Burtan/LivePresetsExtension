genversion_src = vcs_tag(
    command: [find_program('git'), 'describe', '--tags', '--dirty'],
    input: 'genversion.cpp.in',
    output: 'genversion.cpp',
    fallback: '0.0-nogit'
)
genversion = executable('genversion', genversion_src, native: native)

project_sources += custom_target('version.h',
                        command: [genversion],
                        capture: true,
                        output: 'version.h'
)
