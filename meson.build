project('LivePresetsExtension', 'cpp',
        version : '1.0',
        license : [ 'MIT'],
        meson_version : '>= 0.50.0',
        default_options : [ 'warning_level=3', 'buildtype=debugoptimized', 'cpp_std=c++20' ]
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
# This links all the static libs into the main source file to form a binary

sample_cpp_project_bin = shared_library('livepresets',
                                    project_sources,
                                    include_directories : inc)

gtest = subproject('gtest')
benchmark = subproject('benchmark')

if get_option('enable-tests')
    subdir('tests')
endif
if get_option('enable-benchmarks')
    subdir('benchmarks')
endif

# This adds the clang format file to the build directory
configure_file(input : '.clang-format',
               output : '.clang-format',
               copy: true)
run_target('format',
           command : ['clang-format','-i','-style=file',project_sources,project_test_sources,project_benchmark_sources,project_header_files])

# This regex excludes any sources from the third_party, tests, benchmarks
# and gtest related files.
regex = '^((?!(third_party|tests|benchmarks|gtest)).)*$'
# This adds clang tidy support
configure_file(input : '.clang-tidy',
               output : '.clang-tidy',
               copy : true)
# run_target('tidy', command : ['run-clang-tidy.py','-fix', '-j' , '8', 'files', regex ,'-format', '-p='+ meson.build_root()])

