cmake_minimum_required(VERSION 3.26)
project(stereox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER "clang-16")
set(CMAKE_CXX_COMPILER "clang++-16")

# Creates the compile_commands.json used by editors for code completion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Must use GNUInstallDirs to install libraries into correct locations on all platforms:
include(GNUInstallDirs)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Find OpenCV package
find_package(OpenGL REQUIRED)

# Find spdlog package
find_package(spdlog REQUIRED)

# Find GTK-MM package
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

# Find v4l2 (video for linux) package
pkg_check_modules(V4L2 REQUIRED libv4l2)

# Create an executable
add_executable(${PROJECT_NAME}
        src/main.cpp
        src/calibration/ui_calibration.cpp
        src/utils/errors/error_reporter.cpp
        src/camera/stereo_camera.cpp
        src/camera/stereo_camera.h
        src/utils/tp/thread_pool.cpp
        src/utils/tp/thread_pool.h
        src/utils/stb_image.h
        src/ogl/render/texture_1.cpp
        src/ogl/render/texture_1.h
        src/ogl/shader/simple_shader.cpp
        src/ogl/shader/simple_shader.h
        src/gtk/gl_image.cpp
        src/gtk/gl_image.h
        src/utils/loop/delta_loop.cpp
        src/utils/loop/delta_loop.h
        src/v4l2/linux_video.cpp
        src/v4l2/linux_video.h
        src/gtk/gtk_cam_params.cpp
        src/gtk/gtk_cam_params.h
        src/gtk/gtk_utils.h
)

# Include directories for the specific target
target_include_directories(${PROJECT_NAME}
        PRIVATE ${OpenCV_INCLUDE_DIRS}
        PRIVATE ${GTKMM_INCLUDE_DIRS}
        PRIVATE ${V4L2_INCLUDE_DIRS})

# Link libraries
target_link_libraries(${PROJECT_NAME}
        PRIVATE ${OpenCV_LIBS}
        PRIVATE ${GTKMM_LIBRARIES}
        PRIVATE OpenGL::GL
        PRIVATE spdlog::spdlog
        PRIVATE ${V4L2_LIBRARIES})

target_compile_options(${PROJECT_NAME}
        PRIVATE ${GTKMM_CFLAGS_OTHER}
        PRIVATE ${V4L2_CFLAGS_OTHER})