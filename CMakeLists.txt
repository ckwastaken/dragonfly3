cmake_minimum_required(VERSION 3.21)
project(flycompact LANGUAGES C CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../binpack")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../binpack")

add_library(salt SHARED
        src/salt.cpp
        src/buildconfigs.h
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Point to your CEF folder
set(CEF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cef_156")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

find_package(CEF REQUIRED)

# 2. Define the logical target for libcef.so (THIS FIXES THE LINKER ERRORS)
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

# 3. Build the C++ wrapper
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)


# 4. Define your executable
add_executable(${PROJECT_NAME} src/main.cpp
        src/fly_gui.cpp
        src/fly_imageutils.cpp
        src/fly_global_handler.cpp
        src/fly_imageutils.h
        src/fly_gui.h
        src/fly_toast.cpp
        src/fly_toast.h
        src/salt.h
        src/fly_resourcemanager.cpp
        src/CustomResourceHandlerTest.cpp
        src/fly_displayhandler.cpp
        src/fly_rendermanager.cpp
        src/fly_rendermanager.h
        src/fly_processmessage.cpp
        src/fly_processmessage.h
        src/keybinds.cpp
        src/keybinds.h
        src/global.h
)

target_link_libraries(flycompact PRIVATE salt)

SET_EXECUTABLE_TARGET_PROPERTIES(${PROJECT_NAME})

# Ensure the executable finds libcef.so in the same folder at runtime
set_target_properties(${PROJECT_NAME} PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
)

include_directories(/usr/include/freetype2)
include_directories(/usr/include/X11)

target_include_directories(${PROJECT_NAME} PRIVATE "${CEF_ROOT}")

# 5. Link libcef_lib alongside libcef_dll_wrapper
target_link_libraries(${PROJECT_NAME} PRIVATE
        # third party
        libcef_lib
        libcef_dll_wrapper
        ${CEF_STANDARD_LIBS}
        X11
        Xft
)


# 6. Copy CEF binaries and resources to the build folder
COPY_FILES("${PROJECT_NAME}" "buildconfigs.h" "${CMAKE_CURRENT_SOURCE_DIR}/src" "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
COPY_FILES("${PROJECT_NAME}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
COPY_FILES("${PROJECT_NAME}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
