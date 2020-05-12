/*
FrameProc gets frames from video file and and processes in another thread usingthread-safe queue
*/
	
#include <iostream>
#include <cassert>
#include <cstring>

// thread-safe-queue for processing frames in threads
#include "ThreadSafeQueue.h"

// OpenCV functions
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"

#pragma once

using namespace cv;


class FrameProc {
	string window_name;
	std::shared_ptr<VideoCapture> cap;
	string video_fname;
	bool frames_ended;
	ThreadSafeQueue<Mat> safe_queue;
	int fps;
public:
	FrameProc(const string&, int);
	~FrameProc();

	bool IsVideoCapOk(std::shared_ptr<VideoCapture> cap);
	void ProcessVideo();
	void GetFrame(ThreadSafeQueue<Mat>&);
	void ProcessFrame(ThreadSafeQueue<Mat>&);

	std::thread get_frame_thread() 
	{
		return std::thread([this] 
			{
				GetFrame(std::ref(safe_queue)); 
			});
	}
	
	std::thread process_frame_thread() 
	{
		return std::thread([this] 
			{ 
				ProcessFrame(std::ref(safe_queue)); 
			});
	}


};