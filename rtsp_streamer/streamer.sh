sudo docker run -e INPUT=/tmp/iron-man.mp4 -v /home/ildar/work/opencv-video-sketch/samples/iron-man.mp4:/tmp/iron-man.mp4 -e RTSP_FRAMERATE=20 --rm -p 8555:8554 ullaakut/rtspatt

