# Opencv video processing
A multihreaded console application processing video file using two std::threads.

1. Change directory to CMakeLists.txt file dir. 
2. Type in terminal:<br>
``$cmake .``<br>
``$make``

OpenCV has a number of backends. To change it use ``CAP_GSTREAMER``, ``CAP_FFMPEG`` etc. in <br>``FrameProc *video_proc = new FrameProc(fname, CAP_GSTREAMER);`` on line 16 of ``main.cpp``. The application gets the filepath of the video file as an argument and uses two ``std::thread``: one thread decodes the frames from ``cv::VideoCapture`` and adds to thread-safe-queue, the second thread gets frames from the queue, processes and shows using ``cv::imshow``. Processing is the ``cv::GaussianBlur`` and extracting contours using ``cv::Canny``. Thread write the size of queue to console after ``push()`` and ``pop()`` operations.


The GIF below demostrates reults of video processing.

![OpenCV multithreading demo](https://github.com/integra-dev/opencv-video-sketch/blob/master/gif/opencv-sketch.gif)
