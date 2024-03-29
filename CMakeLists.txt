cmake_minimum_required(VERSION 3.26)
project(stereox VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TFLITE_ENABLE_GPU ON CACHE BOOL "Enable TFLite GPU support")
set(ABSL_PROPAGATE_CXX_STD ON)

add_subdirectory(external/tensorflow/tensorflow/lite)
add_subdirectory(external/argparse)
add_subdirectory(external/glm)

set(CMAKE_C_COMPILER "clang-16")
set(CMAKE_CXX_COMPILER "clang++-16")

# Creates the compile_commands.json used by editors for code completion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Must use GNUInstallDirs to install libraries into correct locations on all platforms:
include(GNUInstallDirs)

# Find OpenCL
find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})

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
        src/cli.h
        src/calibration/ui_calibration.cpp
        src/aux/utils/errors/error_reporter.cpp
        src/aux/ocv/stereo_camera.cpp
        src/aux/ocv/stereo_camera.h
        src/aux/utils/tp/thread_pool.cpp
        src/aux/utils/tp/thread_pool.h
        src/aux/ogl/render/texture_1.cpp
        src/aux/ogl/render/texture_1.h
        src/aux/ogl/shader/simple_shader.cpp
        src/aux/ogl/shader/simple_shader.h
        src/aux/gtk/gl_image.cpp
        src/aux/gtk/gl_image.h
        src/aux/utils/loop/delta_loop.cpp
        src/aux/utils/loop/delta_loop.h
        src/aux/v4l2/linux_video.cpp
        src/aux/v4l2/linux_video.h
        src/aux/gtk/gtk_cam_params.cpp
        src/aux/gtk/gtk_cam_params.h
        src/aux/gtk/gtk_utils.h
        src/aux/utils/globals/eox_globals.cpp
        src/aux/utils/globals/eox_globals.h
        src/aux/gtk/gtk_config_stack.cpp
        src/aux/gtk/gtk_config_stack.h
        src/aux/gtk/gtk_eox_window.cpp
        src/aux/gtk/gtk_eox_window.h
        src/aux/utils/mappers/cam_gtk_mapper.h
        src/aux/commons.h
        src/helpers/helpers.cpp
        src/helpers/helpers.h
        src/aux/utils/timer/timer.cpp
        src/aux/utils/timer/timer.h
        src/calibration/calibration.cpp
        src/aux/ocv/cv_utils.cpp
        src/aux/ocv/cv_utils.h
        src/cloud/ui_points_cloud.cpp
        src/cloud/ui_points_cloud.h
        src/cloud/cloud.cpp
        src/aux/gtk/gtk_control.cpp
        src/aux/gtk/gtk_control.h
        src/aux/utils/mappers/cam_gtk_mapper.cpp
        src/aux/ocv/point_cloud.cpp
        src/aux/ocv/point_cloud.h
        src/aux/ocv/kernel.cpp
        src/aux/ocv/kernel.h
        src/aux/ogl/render/voxels.cpp
        src/aux/ogl/render/voxels.h
        src/aux/ogl/camera/camera.cpp
        src/aux/ogl/camera/camera.h
        src/aux/gtk/gl_voxel_area.cpp
        src/aux/gtk/gl_voxel_area.h
        src/aux/dnn/blaze_pose.cpp
        src/aux/dnn/blaze_pose.h
        src/pose/ui_pose.cpp
        src/pose/ui_pose.h
        src/aux/dnn/dnn_common.h
        src/aux/dnn/dnn_common.cpp
        src/aux/sig/low_pass_filter.cpp
        src/aux/sig/low_pass_filter.h
        src/aux/sig/velocity_filter.cpp
        src/aux/sig/velocity_filter.h
        src/aux/dnn/roi/pose_roi.cpp
        src/aux/dnn/roi/pose_roi.h
        src/aux/dnn/roi/RoiPredictor.h
        src/pipeline/pose_pipeline.cpp
        src/pipeline/pose_pipeline.h
        src/aux/dnn/ssd/ssd_anchors.h
        src/aux/dnn/ssd/ssd_anchors.cpp
        src/aux/dnn/pose_detector.cpp
        src/aux/dnn/pose_detector.h)

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
        PRIVATE ${V4L2_LIBRARIES}
        PRIVATE tensorflow-lite
        PRIVATE argparse
        PRIVATE glm)

target_compile_options(${PROJECT_NAME}
        PRIVATE ${GTKMM_CFLAGS_OTHER}
        PRIVATE ${V4L2_CFLAGS_OTHER})