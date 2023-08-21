cmake_minimum_required(VERSION 3.26)
project(stereox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER "clang-16")
set(CMAKE_CXX_COMPILER "clang++-16")

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Find OpenCV package
find_package(OpenGL REQUIRED)

# Find GTK-MM package
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

# Create an executable
add_executable(${PROJECT_NAME}
        src/main.cpp
        src/calibration/ui_calibration.cpp
        src/calibration/ui_calibration.h
        src/errors/error_reporter.cpp
        src/errors/error_reporter.h
)

# Include directories for the specific target
target_include_directories(${PROJECT_NAME}
        PRIVATE ${OpenCV_INCLUDE_DIRS}
        PRIVATE ${GTKMM_INCLUDE_DIRS})

# Link libraries
target_link_libraries(${PROJECT_NAME}
        PRIVATE ${OpenCV_LIBS}
        PRIVATE ${GTKMM_LIBRARIES}
        PRIVATE OpenGL::GL)

target_compile_options(${PROJECT_NAME}
        PRIVATE ${GTKMM_CFLAGS_OTHER})