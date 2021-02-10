// CPU-accelerated frame decoder based on GStreamer API

#include <cassert>
#include <cstring>
#include <vector>
#include <iostream>

// GStreamer 
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>

// OpenCV functions
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"

#include "ThreadSafeQueue.h"

using namespace std;
using namespace cv;

#pragma once


GstFlowReturn new_preroll(GstAppSink* appsink, gpointer data);

GstFlowReturn new_sample(GstAppSink* appsink, gpointer data);

// bus callback important messages - should be static
static gboolean my_bus_callback(GstBus* bus, GstMessage* message, gpointer data);


class FrameDecoder
{
private:
	GstElement* pipeline, * bin;
	GstElement* sink;
	int gdec_fps;
public:
	FrameDecoder(int argc, char* argv[], int);
	~FrameDecoder();

	bool TestElement(const string&);
	bool TestAllElementsInstalled();
	void CreatePipeLine(const string&);
	void ClearPipeLine();
	void LoadVideoFile(const string&);
};


// FrameProc processes frames from thread-safe-queue in another thread usingthread-safe queue
class FrameProc {
	string window_name;
	std::shared_ptr<VideoCapture> cap;
	std::shared_ptr<FrameDecoder> g_decoder;
	string video_fname;
	int fps;
public:
	FrameProc(const string&, int);
	~FrameProc();

	void InitGDecoder(int argc, char* argv[]);
	bool IsVideoCapOk(std::shared_ptr<VideoCapture> cap);
	void ProcessVideo();
	void GetFrame();
	void ProcessFrame();

	std::thread get_frame_thread() 
	{
		return std::thread([this] 
			{
				GetFrame(); 
			});
	}
	
	std::thread process_frame_thread() 
	{
		return std::thread([this] 
			{ 
				ProcessFrame(); 
			});
	}
};