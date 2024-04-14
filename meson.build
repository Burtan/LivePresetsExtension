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

inc = [include_directories('include')]

subdir('include')
subdir('third_party')
# This is where you should add in include directories

# This triggers the builds of sources
subdir('liblpe')

gtest = subproject('gtest')
benchmark = subproject('benchmark')
wdl = subproject('wdl')

#depWdl = dependency('wdl-1.0')

if get_option('enable-tests')
    subdir('tests')
endif
if get_option('enable-benchmarks')
    subdir('benchmarks')
endif

livepresets_bin = shared_library('livepresets',
                                        project_sources,
                                        include_directories : inc,
                                        #dependencies: [depWdl]
)
#
## This adds the clang format file to the build directory
#configure_file(input : '.clang-format',
#               output : '.clang-format',
#               copy: true)
#run_target('format',
#           command : ['clang-format','-i','-style=file',project_sources,project_test_sources,project_benchmark_sources,project_header_files])
#
## This regex excludes any sources from the third_party, tests, benchmarks
## and gtest related files.
#regex = '^((?!(third_party|tests|benchmarks|gtest|wdl)).)*$'
## This adds clang tidy support
#configure_file(input : '.clang-tidy',
#               output : '.clang-tidy',
#               copy : true)
## run_target('tidy', command : ['run-clang-tidy.py','-fix', '-j' , '8', 'files', regex ,'-format', '-p='+ meson.build_root()])
#
