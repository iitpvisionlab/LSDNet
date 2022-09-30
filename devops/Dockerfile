FROM nvidia/cuda:11.3.0-cudnn8-devel-ubuntu20.04

# install system
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install --fix-missing -y \
    build-essential \
    libatlas-base-dev \
    libavcodec-dev \
    libavformat-dev \
    libavdevice-dev \
    libfontconfig1 \
    libgtk-3-dev \
    libjpeg-dev \
    libpng-dev \
    libsm6 \
    libswscale-dev \
    libtiff-dev \
    libv4l-dev \
    libx264-dev \
    libxext6 \
    libxrender1 \
    libxvidcore-dev \
    yasm \
    python3-dev \
    locales \
    sudo \
    ninja-build \
    ffmpeg \
    wget \
    unzip \
    python3-pip \
    && \
    rm -rf /var/lib/apt/lists/* \
    && \
    locale-gen "en_US.UTF-8"

# cmake
RUN wget https://github.com/Kitware/CMake/releases/download/v3.19.7/cmake-3.19.7-Linux-x86_64.sh && \
    sh cmake-3.19.7-Linux-x86_64.sh --skip-license --prefix=/usr/local

# install opencv
ARG opencv_ver=4.5.2
RUN wget -O opencv-${opencv_ver}.zip https://github.com/opencv/opencv/archive/${opencv_ver}.zip && \
    wget -O opencv_contrib-${opencv_ver}.zip https://github.com/opencv/opencv_contrib/archive/${opencv_ver}.zip && \
    unzip opencv-${opencv_ver}.zip && \
    unzip opencv_contrib-${opencv_ver}.zip && \
    mkdir opencv-${opencv_ver}/build && cd opencv-${opencv_ver}/build && \
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D INSTALL_PYTHON_EXAMPLES=ON \
    -D INSTALL_C_EXAMPLES=OFF \
    -D OPENCV_ENABLE_NONFREE=ON \
    -D WITH_OPENCL=OFF \
    -D WITH_OPENJPEG=OFF \
    -D WITH_JASPER=OFF \
    -D WITH_OPENEXR=OFF \
    -D WITH_V4L=OFF \
    -D WITH_FFMPEG=OFF \
    -D WITH_GSTREAMER=OFF \
    -D BUILD_JAVA=OFF \
    -D WITH_CUDA=OFF \
    -D BUILD_TESTS=OFF \
    -D WITH_CUDNN=OFF \
    -D OPENCV_DNN_CUDA=OFF \
    -D OPENCV_EXTRA_MODULES_PATH=/opencv_contrib-${opencv_ver}/modules \
    -D HAVE_opencv_python3=ON \
    -D BUILD_EXAMPLES=OFF .. && \
    make install -j12 && \
    ldconfig && \
    rm -rf opencv-${opencv_ver}.zip opencv_contrib-${opencv_ver}.zip \
    opencv-${opencv_ver} opencv_contrib-${opencv_ver}

# python
RUN ln -s /usr/bin/python3 /usr/local/bin/python
# RUN python -m pip --upgrade pip --no-cache-dir
COPY requirements.txt .
RUN pip3 install -r requirements.txt --no-cache-dir
