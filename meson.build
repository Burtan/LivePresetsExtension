project('LivePresetsExtension', 'cpp',
        license: ['MIT'],
        meson_version: '>= 1.4.0',
        default_options: [
            'b_lto=true', # -fno-lto added below in debug builds
            'b_ndebug=if-release',
            'b_pie=true',
            'b_vscrt=static_from_buildtype',
            'cpp_std=c++20',
            'build.cpp_std=c++20',
            'cpp_winlibs=', # explicitely specified per-target
            'default_library=static',
            'warning_level=2',
            'WDL:warning_level=0',
            'werror=true',
        ]
)

# compiler options
global_cpp_args = {
    'gcc': [
        '-fsigned-char', # defaults to unsigned on ARM
        '-fstack-protector-strong',
        '-fvisibility=hidden', # meson's gnu_symbol_visibility is per-target
    ],
    'msvc': [],
}
project_cpp_args = {
    'gcc': [
        '-Wno-deprecated-copy', # for eel_strings.h in GCC 12
        '-Wno-missing-braces',  # for Xcode 9
        '-Wno-missing-field-initializers',
        '-Wno-multichar',
        '-Wno-nonnull-compare',  # dynamic_cast(this)
        '-Wno-unused-parameter', # for reaper-sdk and WDL
    ],
    'msvc': [
        '/D_CRT_NONSTDC_NO_DEPRECATE',
        '/D_CRT_SECURE_NO_WARNINGS',
        '/DNOMINMAX',
        #'/DUNICODE',

        '/wd4244', # conversion from T1 to T2, possible loss of data
        '/wd4267', # conversion from size_t to T, possible loss of data
        '/wd4624', # destructor was implicitly defined as deleted
    ],
}
project_objcpp_args = [
    '-fobjc-arc',
    '-Wno-deprecated-declarations',
]
link_args = {
    'gcc': [
        # remove unused code (-dead_strip on macOS)
        '-Wl,-dead_strip', '-Wl,--gc-sections',
        # fail at compile-time if we forgot to link against something
        '-Wl,--no-undefined',
    ],
    'msvc': [
        # store only the PDB file's basename instead of its full absolute path
        '/PDBALTPATH:%_PDB%'
    ]
}

if get_option('buildtype') == 'debug'
    global_cpp_args += {'gcc':  global_cpp_args['gcc'] + [
                            '-fno-lto' # meson is lacking a b_lto=if-release option
                        ]}
else
    global_cpp_args += {'msvc':  global_cpp_args['msvc'] + [
                            '/GL', # whole program optimization
                            '/Gy', # enable function-level linking
                            '/Zc:inline', # remove unreferenced COMDAT
                            '/Zi', # generate a PDB file (implies /DEBUG and /Zo)
                        ]}
    link_args += {'msvc':  link_args['msvc'] + [
                      '/LTCG',
                      '/OPT:REF,ICF,LBR', # re-enable link-time optimizations when /DEBUG is set
                  ]}
endif

native_set = [false]
if meson.is_cross_build()
    # not copying compiler flags to the native compiler unless in a cross build
    # to avoid unnecessary duplicate supported argument check status messages
    native_set += true
endif
foreach native : native_set
    cpp = meson.get_compiler('cpp', native: native)
    arg_syntax = cpp.get_argument_syntax()
    add_global_arguments(
        cpp.get_supported_arguments(global_cpp_args[arg_syntax]),
        language: ['c', 'cpp', 'objcpp'],
        native: native
    )
    add_project_arguments(
        cpp.get_supported_arguments(project_cpp_args[arg_syntax]),
        language: ['c', 'cpp', 'objcpp'],
        native: native
    )
    add_global_link_arguments(
        cpp.get_supported_link_arguments(link_args[arg_syntax]),
        language: ['c', 'cpp', 'objcpp'],
        native: native
    )
endforeach

if host_machine.system() == 'darwin'
    add_languages('objcpp', native: false)
    objcpp = meson.get_compiler('objcpp')
    add_project_arguments(
        objcpp.get_supported_arguments(project_objcpp_args),
        language: 'objcpp')
endif

# dependencies
win_dep = []
if host_machine.system() == 'windows'
    foreach win32_lib : ['User32', 'Gdi32', 'Advapi32', 'Comdlg32', 'Shell32']
        win_dep += cpp.find_library(win32_lib)
    endforeach
endif
wdl_dep = dependency('WDL')
reaper_sdk_dep = dependency('reaper-sdk')

if host_machine.system() == 'windows'
    swell_dep = dependency('', required: false)
else
    swell_dep = dependency('SWELL')
endif

# sources
project_sources = []
project_header_files = []
project_test_sources = []
project_benchmark_sources = []

subdir('third_party')
subdir('liblpe')
subdir('tools')

gtest = subproject('gtest')
#benchmark = subproject('benchmark')
wdl = subproject('WDL')
subproject('reaper-sdk')

if get_option('enable-tests')
    subdir('tests')
endif
if get_option('enable-benchmarks')
    subdir('benchmarks')
endif

# installation
fs = import('fs')
arch_suffix = host_machine.cpu()
if host_machine.system() == 'windows' and arch_suffix == 'x86_64'
    arch_suffix = 'x64'
endif

install_platform = build_machine.system()
if install_platform == 'darwin'
    resource_path = fs.expanduser('~/Library/Application Support')
elif install_platform == 'windows'
    appdata = run_command('cmd', '/C', 'echo %APPDATA%', check: true)
    resource_path = appdata.stdout().strip()
else
    resource_path = fs.expanduser('~/.config')
endif
resource_path = resource_path / 'REAPER'
plugins_dir = resource_path / 'UserPlugins'

livepresets = shared_library('livepresets_' + arch_suffix, project_sources,
                             dependencies: [wdl_dep, swell_dep, win_dep, reaper_sdk_dep],
                             name_prefix: 'reaper_',
                             install: true,
                             install_dir: plugins_dir
)