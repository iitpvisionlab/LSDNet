cmake_minimum_required(VERSION 3.17)
project(Line_extracter)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(Line_extracter main.cpp compute_eigen_coords.cpp compute_eigen_coords.h
        img_processor.cpp img_processor.h neural_network.cpp neural_network.h region_growing.cpp
        region_growing.h region_splitter.cpp region_splitter.h roi_extraction.cpp roi_extraction.h
        segment_extractor.cpp segment_extractor.h to_original_scale.cpp to_original_scale.h kde.h kde.cpp
        lsd_methods.cpp lsd_methods.h)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})