project(
    'flycompact',
    ['c', 'cpp'],
    default_options: [
        'cpp_std=c++20',
        'cpp_std=gnu++20',
        'c_std=c11',
        'warning_level=1',
    ]
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

cef_root = meson.current_source_dir() / 'third_party/cef_156'
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
    dependency('freetype2')
]

cef_inc = include_directories(
    'third_party/cef_156',
    'third_party/cef_156/include',
    'third_party/cef_156/libcef_dll',
)

# --------------------------------------------------
# CEF library
# --------------------------------------------------

cef_lib = cpp.find_library(
    'cef',
    dirs: cef_release,
    required: true,
)

# --------------------------------------------------
# CEF DLL wrapper
# --------------------------------------------------

cef_wrapper = cef_root / '../libcef_dll_wrapper.a'
cef_wrapper_path = cef_root / '../libcef_dll_wrapper.a'

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
# Main executable
# --------------------------------------------------

copy_cef = custom_target(
    'copy-cef',
    output: 'cef_copied.stamp',
    command: [
        'bash',
        '-c',
        'mkdir -p "$1" && cp -r "$2"/* "$1"/ && cp -r "$3"/* "$1"/ && touch "$4"',
        'copy-cef',
        meson.current_build_dir(),
        cef_release,
        cef_resources,
        '@OUTPUT@',
    ],
    build_by_default: true,
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
        include_directories(
            '/usr/include/freetype2',
            '/usr/include/X11',
        ),
    ],

    link_with: [
        salt,
        twlconnector,
    ],

    dependencies: cef_deps,

    link_args: [
        cef_wrapper_path,
        '-L' + cef_release,
        '-lcef',
        '-Wl,-rpath,$ORIGIN',
    ],

    install: false,
)

# --------------------------------------------------
# Copy CEF runtime files
# --------------------------------------------------

run_target(
    'copy-cef',
    command: [
        'bash',
        '-c',
        'mkdir -p "$1" && cp -a "$2"/. "$1"/ && cp -a "$3"/. "$1"/',
        'copy-cef',
        meson.current_build_dir(),
        cef_release,
        cef_resources,
    ],
)
