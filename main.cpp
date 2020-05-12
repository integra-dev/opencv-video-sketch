// C++11
// OpenCV 4.1.0 used for testing
// Using CAP_GSTREAMER by default to decode frames from video file

#include "FrameProc.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("usage: main.out <video_file_path> \n");
        return -1;
    }

    string fname = argv[1]; "samples/iron-man.mp4"; 
    FrameProc *video_proc = new FrameProc(fname, CAP_GSTREAMER);    // CAP_FFMPEG, CAP_GSTREAMER availlable
    video_proc->ProcessVideo();  // starts two threads and joins them further

    return 0;
}