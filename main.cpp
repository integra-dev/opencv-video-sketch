// C++11
// OpenCV 4.1.0 used for testing
// Using CAP_GSTREAMER by default to get FPS parameter and gstreamer-1.0 to decode frames from video file

#include "FrameProc.h"

int main(int argc, char* argv[])
{
   if (argc != 2)
    {
        cout << "Usage: main.out <video_file_path> \n";
        return -1;
   }

<<<<<<< HEAD
//    g_main_iteration(false);
=======
>>>>>>> ced276223ef43def846c8727b543faecb3cb6b19
    string fname = argv[1];  // "samples/iron-man.mp4";
    std::shared_ptr<FrameProc> video_proc = std::make_shared<FrameProc>(fname, CAP_GSTREAMER);    // CAP_FFMPEG, CAP_GSTREAMER availlable
    video_proc->InitGDecoder(argc, argv);
    video_proc->ProcessVideo();         // starts two threads and joins them further

    return 0;
}
