#include "FrameProc.h"

#define NDEBUG


// global container to pass both to main thread and GStreamer thread
ThreadSafeQueue<Mat> safe_queue;
bool frames_ended(false);

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
    cap->release();

    window_name = "opencv-sketch TEST CASE - Canny edges";
}

void FrameProc::InitGDecoder(int argc, char* argv[])
{
    g_decoder = std::make_shared<FrameDecoder>(argc, argv, fps);
    g_decoder->TestAllElementsInstalled();
}
           
bool FrameProc::IsVideoCapOk(std::shared_ptr<VideoCapture> cap)
{
    // if not success, exit program
    if (cap->isOpened() == false)
    {
        cerr << "Cannot open the video file!" << endl;
        cin.get();      // wait for any key press and exit
        return false;
    }
    return true;
}


void FrameProc::ProcessVideo()
{
    auto get_thread = get_frame_thread();
    ProcessFrame();       // starting processing and visualizing in main thread
    //auto process_thread = process_frame_thread();

    if (get_thread.joinable()) get_thread.join();
    //if (process_thread.joinable()) process_thread.join();
}


void FrameProc::GetFrame()
{
    g_decoder->LoadVideoFile(video_fname);
}


void FrameProc::ProcessFrame()
{
    while (true)
    {
		g_main_iteration(false);
        if (frames_ended)   // the last frame was decoded
        {
			g_decoder->ClearPipeLine();
            break;
        }
        while (!safe_queue.is_empty())      // extracting frames from queue
        {
            Mat frame = safe_queue.pop();
            cout << "Item popped: safe_queue.size=" << safe_queue.get_size() << endl;

            if (frame.empty())  // avoid processing empty frames
                continue;

            Mat edges;      // new frame to write found edges in source frameprocessing

            // tranforming frame to grayscale image
            cvtColor(frame, edges, COLOR_BGR2GRAY);

            // Gaussian Blur processing to reduce noise and 
            GaussianBlur(edges, edges, Size(7, 7), 1.0, 1.0);       
            Canny(edges, edges, 0, 30, 3, true);

            //show frame in created window
            namedWindow(window_name, WINDOW_FREERATIO);
            imshow(window_name, edges);

            waitKey(fps);
        }
    }
}

FrameProc::~FrameProc()
{
}



// ------- FrameDecoder ---------
FrameDecoder::FrameDecoder(int argc, char* argv[], int fps) :
	gdec_fps(fps)
{
	// GStreamer initialization
	gst_init(&argc, &argv);

	const gchar* nano_str;
	guint major, minor, micro, nano;
	gst_version(&major, &minor, &micro, &nano);

	if (nano == 1)
		nano_str = "(CVS)";
	else if (nano == 2)
		nano_str = "(Pre-Release)";
	else
		nano_str = "";

	g_print("GStreamer version: %d.%d.%d %s\n", major, minor, micro, nano_str);
}


bool FrameDecoder::TestElement(const string& el_str)
{
	const gchar* elem_name = el_str.c_str();
	GstElementFactory* factory;

	/* get factory */
	factory = gst_element_factory_find(elem_name);
	if (!factory) {
		cerr << "You don't have the " << "\'" << string(elem_name) << "\'" << " element installed!" << endl;
		return false;
	}
	return true;
}

bool FrameDecoder::TestAllElementsInstalled()
{
	bool all_installed = true;

	std::vector<string> elements;
	elements.push_back("filesrc");
	elements.push_back("decodebin");
	elements.push_back("queue");
	elements.push_back("videoconvert");
	elements.push_back("appsink");
	// elements.push_back("autovideosink");
	// elements.push_back("avdec_h264");  // codec plugin for mp4 - a part of libav/

	for (auto& el : elements)
		all_installed = TestElement(el);

	return all_installed;
}

void FrameDecoder::CreatePipeLine(const string& video_path)
{
	string config = "";
	config.append("filesrc location=").append(video_path.c_str()).append(" ")
		.append("! decodebin ! queue ! videoconvert ! video/x-raw,format=BGR ")
		.append("! appsink name=sink emit-signals=true sync=true max-buffers=1");// .append(std::to_string(gdec_fps));
	// if sync=true => CPU usage is 320%
	// if sync=true => CPU usage is only 9.3%. In this case we can use safe-thread-queue to pull and render frame from video stream

	gchar* descr = g_strdup(config.c_str());

	g_print("GStreamer pipeline configuration:\n%s\n", descr);
	g_print("Creating pipeline...\n");

	// Create and check pipeline
	GError* error = nullptr;
	pipeline = gst_parse_launch(descr, &error);

	if (error)
	{
		g_print("Could not construct pipeline: %s\n", error->message);
		g_error_free(error);
		exit(-1);
	}
	g_print("Adding elements...\n");

	// Get sink for frames output
	sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

	// Get sink signals and check for a preroll
	// If preroll exists, we do have a new frame
	gst_app_sink_set_emit_signals((GstAppSink*)sink, true);
	gst_app_sink_set_drop((GstAppSink*)sink, true);
	gst_app_sink_set_max_buffers((GstAppSink*)sink, 1 /*gdec_fps*/);
	GstAppSinkCallbacks callbacks = { nullptr, new_preroll, new_sample };
	gst_app_sink_set_callbacks(GST_APP_SINK(sink), &callbacks, nullptr, nullptr);

	// Declare bus
	GstBus* bus;
	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	gst_bus_add_watch(bus, my_bus_callback, nullptr);
	gst_object_unref(bus);

	g_print("\Pipeline is ready!\n");

	// starting pipeline
	gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
}

void FrameDecoder::ClearPipeLine()
{
	// unref gstreamer pipeline
	gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(pipeline));
}

void FrameDecoder::LoadVideoFile(const string& video_path)
{
	bool isOk = TestAllElementsInstalled();
	if (isOk)
		g_print("\nAll GStreamer elements installed for decoding!\n");
	CreatePipeLine(video_path);
}

FrameDecoder::~FrameDecoder()
{
}


// A callback getting a new frame when a preroll exists
// this method cannot be a member of class
GstFlowReturn new_sample(GstAppSink* appsink, gpointer data)
{
	// Get caps and frame
	GstSample* sample = gst_app_sink_pull_sample(appsink);
	GstCaps* caps = gst_sample_get_caps(sample);
	GstBuffer* buffer = gst_sample_get_buffer(sample);
	GstStructure* structure = gst_caps_get_structure(caps, 0);
	const int width = g_value_get_int(gst_structure_get_value(structure, "width"));	// frame width
	const int height = g_value_get_int(gst_structure_get_value(structure, "height")); // frame height
	
																					  //cout << "\nwidth: " << width << " height: " << height << endl;
	// Get frame data
	GstMapInfo map;
	gst_buffer_map(buffer, &map, GST_MAP_READ);

	// Convert gstreamer data to OpenCV Mat
	Mat frame_to_push(cv::Size(width, height), CV_8UC3, (char*)map.data, cv::Mat::AUTO_STEP);

	safe_queue.push(frame_to_push);

	cout << "Item pushed: safe_queue.size = " <<  safe_queue.get_size() << endl;

	gst_buffer_unmap(buffer, &map);
	gst_sample_unref(sample);

	return GST_FLOW_OK;
}


// Print important messages - should be static
static gboolean my_bus_callback(GstBus* bus, GstMessage* message, gpointer data)
{
	// Debug message
	//g_print("Got %s message\n", GST_MESSAGE_TYPE_NAME(message));
	switch (GST_MESSAGE_TYPE(message)) {
	case GST_MESSAGE_ERROR: {
		GError* err;
		gchar* debug;
		gst_message_parse_error(message, &err, &debug);
		g_print("Error: %s\n", err->message);
		g_error_free(err);
		g_free(debug);
		break;
	}
	case GST_MESSAGE_EOS:
		g_print("\nEnd of video stream!\n");
		frames_ended = true;
		break;
	default:
		break;
	}
	/* we want to be notified again the next time there is a message
	 * on the bus, so returning TRUE (FALSE means we want to stop watching
	 * for messages on the bus and our callback should not be called again)
	 */
	return true;
}

// Check preroll to get a new frame using callback https://gstreamer.freedesktop.org/documentation/design/preroll.html
GstFlowReturn new_preroll(GstAppSink* /*appsink*/, gpointer /*data*/)
{
	// if not GST_FLOW_OK is returned than preroll doesn't exist, 
	// so decoder can't got a new frame from source
	return GST_FLOW_OK;
}