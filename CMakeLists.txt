# cmake needs this line
cmake_minimum_required(VERSION 3.0)

# Define project name
project(opencv-sketch)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

find_package(PkgConfig REQUIRED)

set(GSTREAMER_MINIMUM_VERSION 1.0.5)

pkg_check_modules(GST1_TEST gstreamer-1.0)
if ( GST1_TEST_FOUND AND NOT ${GST1_TEST_VERSION} VERSION_LESS ${GSTREAMER_MINIMUM_VERSION} )
    pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
    pkg_check_modules(GSTREAMER-APP REQUIRED gstreamer-app-1.0)
    pkg_check_modules(GSTREAMER-AUDIO REQUIRED gstreamer-audio-1.0)
    pkg_check_modules(GSTREAMER-PBUTILS REQUIRED gstreamer-pbutils-1.0)
    pkg_check_modules(GSTREAMER-FFT REQUIRED gstreamer-fft-1.0)
    add_definitions(-DGST_API_VERSION_1=1)
else()
    # fallback to gstreamer-0.10
    unset(GSTREAMER_MINIMUM_VERSION)
    pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10)
    pkg_check_modules(GSTREAMER-APP REQUIRED gstreamer-app-0.10)
    pkg_check_modules(GSTREAMER-AUDIO REQUIRED gstreamer-audio-0.10)
    pkg_check_modules(GSTREAMER-FFT REQUIRED gstreamer-fft-0.10)
    set_source_files_properties(WebKitWebAudioSourceGStreamer.cpp PROPERTIES COMPILE_DEFINITIONS "GLIB_DISABLE_DEPRECATION_WARNINGS=1")
endif()

set(opencv-sketch_INCLUDE_DIRS
        ${GSTREAMER_INCLUDE_DIRS}
        ${GSTREAMER-APP_INCLUDE_DIRS}
        ${GSTREAMER-AUDIO_INCLUDE_DIRS}
        ${GSTREAMER-PBUTILS_INCLUDE_DIRS}
        ${GSTREAMER-FFT_INCLUDE_DIRS}
)


set(opencv-sketch_LIBRARIES
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER-APP_LIBRARIES}
        ${GSTREAMER-AUDIO_LIBRARIES}
        ${GSTREAMER-PBUTILS_LIBRARIES}
        ${GSTREAMER-FFT_LIBRARIES}
        )

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV REQUIRED)
find_package(Threads QUIET)

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# Declare the executable target built from your sources
add_executable(opencv-sketch main.cpp FrameProc.cpp)

target_include_directories(opencv-sketch PRIVATE ${opencv-sketch_INCLUDE_DIRS})

# Link your application with OpenCV libraries
target_link_libraries(opencv-sketch PRIVATE ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${opencv-sketch_LIBRARIES})