# Opencv video processing
A multithreaded console application processing video file using std::thread to push frames to thread-safe-queue (using __GStreamer__ library) and main thread to process and render popped frames (using __OpenCV__).

1. Change directory to CMakeLists.txt file dir. 
2. Type in terminal:<br>
``$cmake .``<br>
``$make``

OpenCV has a number of backends. To change it use ``CAP_GSTREAMER``, ``CAP_FFMPEG`` etc. in <br>``FrameProc *video_proc = new FrameProc(fname, CAP_GSTREAMER);`` on line 17 of ``main.cpp``. The application gets the filepath of the video file as an argument and uses ``std::thread`` to decode frames with gstreamer API and add to thread-safe-queue, the main thread pops frames from the queue, processes and shows using ``cv::imshow``. Processing is the ``cv::GaussianBlur`` and extracting contours using ``cv::Canny``. Application writes the size of queue to console after ``push()`` and ``pop()`` operations. CPU usage in current configuration is estimated as 9.3% (in case to render extracted frames properly). If gstreamer pipeline contains ``sync=false`` than CPU usage is estimated to 320% and frames are decoded much faster.


For documentation of GStreamer in Russian see [here](http://rus-linux.net/MyLDP/BOOKS/gstreamer/10-helloworld.html).


The GIF below demostrates results of video processing.

<img alt="OpenCV multithreading demo" src="https://github.com/integra-dev/opencv-video-sketch/blob/master/gif/opencv-sketch-min.gif" width="600">
