project('LivePresetsExtension', 'cpp',
        version : '1.0',
        license : ['MIT'],
        meson_version : '>= 1.4.0',
        default_options : [ 'warning_level=0', 'buildtype=debugoptimized', 'cpp_std=c++20' ]
)

# Variables tracking sources and libraries
library_path = []
project_sources = []
project_header_files = []
project_test_sources = []
project_benchmark_sources = []

subdir('third_party')
subdir('liblpe')

gtest = subproject('gtest')
benchmark = subproject('benchmark')
wdl = subproject('WDL')

if get_option('enable-tests')
    subdir('tests')
endif
if get_option('enable-benchmarks')
    subdir('benchmarks')
endif

wdl_dep = dependency('WDL')

if host_machine.system() == 'windows'
    swell_dep = dependency('', required: false)
else
    swell_dep = dependency('SWELL')
endif

livepresets = shared_library(
    'livepresets',
    project_sources,
    dependencies: [wdl_dep, swell_dep]
)