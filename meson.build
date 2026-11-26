project(
    'flycompact',
    ['c', 'cpp'],
    default_options: [
        'cpp_std=gnu++20',
        'c_std=c11',
        'warning_level=1',
    ],
)

add_project_arguments(
    [
        '-DWLR_USE_UNSTABLE',
    ],
    language: 'c',
)

# --------------------------------------------------
# Paths
# --------------------------------------------------

# cef_rel_root = 'third_party/cef_156'
# moved to opt to not have to redownload often

cef_rel_root = '/opt/cef_156'

cef_root = meson.current_source_dir() / cef_rel_root
cef_release = cef_root / 'Release'
cef_resources = cef_root / 'Resources'

# --------------------------------------------------
# Dependencies
# --------------------------------------------------

cpp = meson.get_compiler('cpp')

cef_deps = [
    dependency('threads'),
    dependency('x11'),
    dependency('xft'),
    dependency('freetype2'),
]

cef_inc = include_directories(
    cef_rel_root,
    cef_rel_root / 'include',
    cef_rel_root / 'libcef_dll',
)

# --------------------------------------------------
# CEF prebuilt shared library (libcef.so)
# --------------------------------------------------

cef_lib = cpp.find_library(
    'cef',
    dirs: [cef_release],
    required: true,
)

# --------------------------------------------------
# CEF DLL wrapper (built from source by Meson)
# --------------------------------------------------

py_script = '''
import re, sys
cmake_path, cef_rel_root = sys.argv[1], sys.argv[2]
content = open(cmake_path, encoding='utf-8').read()

wanted = [
    'LIBCEF_SRCS', 'LIBCEF_SRCS_LINUX',
    'LIBCEF_BASE_SRCS',
    'LIBCEF_CPPTOC_SRCS', 'LIBCEF_CPPTOC_SRCS_LINUX',
    'LIBCEF_CTOCPP_SRCS', 'LIBCEF_CTOCPP_SRCS_LINUX',
    'LIBCEF_WRAPPER_SRCS', 'LIBCEF_WRAPPER_SRCS_LINUX',
]

sources = []
for block in wanted:
    m = re.search(r'set\s*\(\s*' + block + r'\b(.*?)\)', content, re.DOTALL)
    if m:
        for token in m.group(1).split():
            if token.endswith(('.cc', '.cpp', '.c')):
                sources.append(f'{cef_rel_root}/libcef_dll/{token}')

print(';'.join(sources))
'''

res = run_command(
    'python3',
    '-c',
    py_script,
    cef_root / 'libcef_dll' / 'CMakeLists.txt',
    cef_rel_root,
    check: true,
)

cef_wrapper_sources = files(res.stdout().strip().split(';'))

libcef_dll_wrapper = static_library(
    'cef_dll_wrapper',
    cef_wrapper_sources,
    include_directories: cef_inc,
    cpp_args: [
        '-DWRAPPING_CEF_SHARED',
        '-Wno-attributes',
        '-Wno-deprecated-declarations',
        '-Wno-unused-parameter',
    ],
)

# --------------------------------------------------
# salt
# --------------------------------------------------

salt = shared_library(
    'salt',
    sources: [
        'src/salt.cpp',
        'src/buildconfigs.h',
    ],
    install: false,
)

# --------------------------------------------------
# twlconnector
# --------------------------------------------------

wlroots = dependency('wlroots-0.20')
wayland_server = dependency('wayland-server')
xkbcommon = dependency('xkbcommon')

twlconnector = shared_library(
    'twlconnector',
    sources: [
        'src/twlconnector.c',
        'src/twlconnector.h',
    ],
    dependencies: [wlroots, wayland_server, xkbcommon],
    install: false,
)

# --------------------------------------------------
# Copy CEF runtime files
# --------------------------------------------------

copy_cef = custom_target(
    'copy-cef',
    output: 'cef_copied.stamp',
    command: [
        'bash',
        '-c',
        'mkdir -p "$1" && cp -r "$2"/. "$1"/ && cp -r "$3"/. "$1"/ && touch "$4"',
        'copy-cef',
        meson.current_build_dir(),
        cef_release,
        cef_resources,
        '@OUTPUT@',
    ],
    build_by_default: true,
)

run_target(
    'copy-cef-manual',
    command: [
        'bash',
        '-c',
        'mkdir -p "$1" && cp -r "$2"/. "$1"/ && cp -r "$3"/. "$1"/',
        'copy-cef-manual',
        meson.current_build_dir(),
        cef_release,
        cef_resources,
    ],
)

# --------------------------------------------------
# Main executable
# --------------------------------------------------

fly_task_manager = shared_library(
    'ftm',
    sources: [
        'src/fly_task_manager.cpp',
        'src/fly_task_manager.h',
    ],
    include_directories: [
        cef_inc,
    ],
    link_with: [
        libcef_dll_wrapper,
        salt,
        twlconnector,
    ],
    dependencies: [cef_lib] + cef_deps,
    link_args: [
        '-Wl,-rpath,$ORIGIN',
    ],
    install: false,
)

flycompact = executable(
    'flycompact',
    copy_cef,
    sources: [
        'src/main.cpp',
        'src/fly_gui.cpp',
        'src/fly_imageutils.cpp',
        'src/fly_global_handler.cpp',
        'src/fly_toast.cpp',
        'src/fly_resourcemanager.cpp',
        'src/CustomResourceHandlerTest.cpp',
        'src/fly_displayhandler.cpp',
        'src/fly_rendermanager.cpp',
        'src/fly_processmessage.cpp',
        'src/keybinds.cpp',
    ],
    include_directories: [
        cef_inc,
    ],
    link_with: [
        fly_task_manager,
        libcef_dll_wrapper,
        salt,
        twlconnector,
    ],
    dependencies: [cef_lib] + cef_deps,
    link_args: [
        '-Wl,-rpath,$ORIGIN',
    ],
    install: false,
)
