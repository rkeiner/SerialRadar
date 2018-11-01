#include "SerialCamera.h"
//#include <stdafx.h>


#include <iostream>
using namespace cv;

using namespace std;
VideoCapture cap; // open the video camera no. 0
Mat frame;

int SerialCamera::init() {


	cap.open(1);
	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the video cam" << endl;
		return -1;
	}

	double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
	double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

	cout << "Frame size : " << dWidth << " x " << dHeight << endl;

	namedWindow("MyVideo", CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
	cap.read(frame);

		bool bSuccess = cap.read(frame); // read a new frame from video

		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video stream" << endl;
		}

		imshow("MyVideo", frame); //show the frame in "MyVideo" window
		//if (waitKey(30) == 27)
		//{
		//	cout << "Esc key is pressed by the user" << endl;
		//	break;
		//}
	return 0;

}
void SerialCamera::begin()
{
}
void SerialCamera::open(char * usbport)
{
}
int SerialCamera::snap(char * filename)
{

	if (!cap.isOpened())  // if not success, exit program
	{
		cout << " video cam is not open" << endl;
	}
	else {

		cap.read(frame);
		bool bSuccess = cap.read(frame); // read a new frame from video

		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video stream" << endl;
		}
		else {
			imshow("MyVideo", frame); //show the frame in "MyVideo" window
		}
		imwrite(filename, frame);
	}
	return 0;
}
bool SerialCamera::isOpen()
{
	return cap.isOpened();
}
void SerialCamera::record(char *filename)
{
}
void SerialCamera::close()
{

}
