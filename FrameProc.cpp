#include "FrameProc.h"

#define NDEBUG

FrameProc::FrameProc(const string& filename, int backend = CAP_FFMPEG) :
    video_fname(filename)
{
    frames_ended = false;
    // creating a VideoCapture object and opening the input file
    cap = std::make_shared<VideoCapture>(video_fname, backend);
    assert(IsVideoCapOk(cap), "Error when opening video file");

    //cap->set(CAP_PROP_POS_MSEC, 300);     // start the video from the certain moment

    //get the frames rate of the video
    fps = cap->get(CAP_PROP_FPS);
    cout << "Frames per seconds : " << fps << endl;

    window_name = "RoadAR TEST CASE - Canny edges";
}


bool FrameProc::IsVideoCapOk(std::shared_ptr<VideoCapture> cap)
{
    // if not success, exit program
    if (cap->isOpened() == false)
    {
        cerr << "Cannot open the video file" << endl;
        cin.get();      // wait for any key press and exit
        return false;
    }
    return true;
}


void FrameProc::ProcessVideo()
{
    /*thread get_thread(&FrameProc::GetFrame, this, std::ref(safe_queue), std::ref(m_q));
    thread process_thread(&FrameProc::ProcessFrame, this, std::ref(safe_queue), std::ref(m_q));*/

    auto get_thread = get_frame_thread();
    ProcessFrame(safe_queue);       // starting processing and visualozing in main thread
    //auto process_thread = process_frame_thread();

    if(get_thread.joinable()) get_thread.join();
    // if (process_thread.joinable()) process_thread.join();

}


void FrameProc::GetFrame(ThreadSafeQueue<Mat>& safe_queue)
{       
    while (true)
    {
        Mat frame;
        bool bSuccess = cap->read(frame); // read a new frame from video

        //Breaking the while loop at the end of the video
        if (bSuccess == false)
        {
            frames_ended = true;
            cout << "Last video frame" << endl;
            cap->release();
            break;
        }
        safe_queue.push(frame); // push the frame to sample
        cout << "Item pushed: safe_queue.size=" << safe_queue.get_size() << endl;
        // cout << "GetFrame thread_id:" << std::this_thread::get_id() << endl; // uncomment to print thread_id

        //wait for for 10 ms until any key is pressed.  
        //If the 'Esc' key is pressed, break the while loop.
        //If the any other key is pressed, continue the loop 
        //If any key is not pressed withing 10 ms, continue the loop
        if (waitKey(10) == 27)
        {
            cout << "Esc key is pressed by user. Stoppig the video" << endl;
            cap->release();
            break;
        }
    }    
}


void FrameProc::ProcessFrame(ThreadSafeQueue<Mat>& safe_queue)
{
    while (true)
    {
        if (frames_ended)   // the last frame was decoded
            break;
        while (!safe_queue.is_empty())      // extracting frames from queue
        {
            Mat frame = safe_queue.pop();
            cout << "Item popped: safe_queue.size=" << safe_queue.get_size() << endl;
            // cout << "ProcessFrame thread_id:" << std::this_thread::get_id() << endl; // uncomment to print thread_id

            if (frame.empty())  // avoid processing empty frames
                continue;

            Mat edges;      // new frame to write found edges in source frameprocessing

            // tranforming frame to grayscale image
            cvtColor(frame, edges, COLOR_BGR2GRAY);

            // Gaussian Blur processing to reduce noise and 
            GaussianBlur(edges, edges, Size(9, 9), 1.5, 1.5);       
            Canny(edges, edges, 0, 50, 3);

            //show frame in created window
            namedWindow(window_name, WINDOW_FREERATIO);
            imshow(window_name, edges);

            waitKey(1000 / fps);
        }
    }
}

FrameProc::~FrameProc()
{
}