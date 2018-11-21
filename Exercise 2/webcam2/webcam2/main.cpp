#include "opencv/cvaux.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_LENGTH (sizeof(char)*3)
#define ADDRESS       L"127.0.0.1"
#define PORT          (12345) 

int main(int argc, char* argv[])
{
	// UDP
	WSADATA  wsadata;
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	unsigned char data[BUFFER_LENGTH];

	/* Open windows connection */
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		fprintf(stderr, "could not initialise winsock\n");
		return 1;
	}

	// create a udp socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("Couldn't create socket!");
		return 1;
	}

	// setup the target port/address
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);

	// convert IP address string to numerical
	if (1 != InetPton(AF_INET, ADDRESS, &si_other.sin_addr)) {
		printf("InetPton failed!");
		return 1;
	}

    // Create camera capture
    CvCapture* camera = cvCreateCameraCapture(0); // Use camera 0
    double h = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT);
    double w = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH);
 
    CvMemStorage* storage = cvCreateMemStorage(0); // needed for Hough circles
    CvSize size = cvSize(w,h);
    IplImage* hsv_frame    = cvCreateImage(size, IPL_DEPTH_8U, 3);
    IplImage* thresholded  = cvCreateImage(size, IPL_DEPTH_8U, 1);
    IplImage* thresholded2 = cvCreateImage(size, IPL_DEPTH_8U, 1);
 
    // Define thresholds
    CvScalar hsv_min = cvScalar(0, 150, 100, 0);
    CvScalar hsv_max = cvScalar(5, 256, 256, 0);
    CvScalar hsv_min2 = cvScalar(206, 150, 100, 0);
    CvScalar hsv_max2 = cvScalar(256, 256, 256, 0);
 
    // Create windows
    cvNamedWindow("window", 1);
    cvNamedWindow("window2", 1);
    
    while (true) {
        // Read new camera frame
        IplImage* frame = cvQueryFrame(camera);
        if (frame == NULL) {
            continue;
        }
 
        // color detection using HSV
        cvCvtColor(frame, hsv_frame, CV_BGR2HSV);
        
        // to handle color wrap-around, two halves are detected and combined
        cvInRangeS(hsv_frame, hsv_min, hsv_max, thresholded);
        cvInRangeS(hsv_frame, hsv_min2, hsv_max2, thresholded2);
        cvOr(thresholded, thresholded2, thresholded);
 
        // Display image in window
        cvShowImage("window", thresholded);
        cvWaitKey(1);

 
        // hough detector works better with some smoothing of the image
        cvSmooth( thresholded, thresholded, CV_GAUSSIAN, 9, 9 );
        CvSeq* circles = cvHoughCircles(thresholded, storage, CV_HOUGH_GRADIENT, 2, thresholded->height/4, 100, 40, 20, 200);
        
        for (int i = 0; i < circles->total; i++)
        {
            float* p = (float*)cvGetSeqElem( circles, i );
            printf("Ball! x=%f y=%f r=%f\n\r",p[0],p[1],p[2] );
            cvCircle( frame, cvPoint(cvRound(p[0]),cvRound(p[1])),
                     3, CV_RGB(0,255,0), -1, 8, 0 );
            cvCircle( frame, cvPoint(cvRound(p[0]),cvRound(p[1])),
                     cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0 );

			//char const *c = reinterpret_cast<char const *>(p);
			//const char* var = "000";
			if (sendto(s, reinterpret_cast<char const *>(p), sizeof(float)*3, MSG_DONTROUTE, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
			//if (sendto(s, (const char*)data, BUFFER_LENGTH, MSG_DONTROUTE, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
			{
				printf("Couldn't send packet: %s\n", p);
			}
        }
 
       // Display image in window
       cvShowImage("window2", frame);
       cvWaitKey(1);
    }
    
    cvReleaseCapture(&camera);
    return 0;
}