#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <windows.h>
#include "main.h"

using namespace std;
using namespace cv;

//amount of frames per second that needs to be displayed
const int FRAMES_PER_SECOND = 25;

//ROI coordinates
int ROI_x_min = -1;
int ROI_y_min = -1;
int ROI_x_max = -1;
int ROI_y_max = -1;
int mouse_x = -1;
int mouse_y = -1;
bool ROI_is_set = false;

//Window to change blur size and sensitivity with trackers
const string trackbarWindowName = "Trackbars";
//Function gets called everytime a trackbar is changed
void on_trackbar(int, void*) {
	//Does nothing
}

//Screen settings
int x_max = 0;
int y_max = 0;

//standard values
int threshold_value = 64;
int erode_value = 7;
int dilate_value = 17;
int min_area = 500;
int y_line = 150;

//Function for getting the clicks inside image
void setROI(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		//if min is not set
		if (ROI_x_min < 0) 
		{
			ROI_x_min = x;
			ROI_y_min = y;
			cout << "Please click on the left down coordinate of the region of intrest" << endl;
		}
		else if(x >= ROI_x_min && y >= ROI_y_min)
		{
			ROI_x_max = x;
			ROI_y_max = y;
			ROI_is_set = true;
		}
	}
	else if (event == EVENT_MOUSEMOVE)
	{
		mouse_x = x;
		mouse_y = y;

	}
}
cv::Rect getROI(Mat Frame) {
	//Open a window with the first frame
	imshow("Select ROI", Frame);
	//Let function set the ROI variables
	setMouseCallback("Select ROI", setROI, NULL);
	//Tell user what to do
	cout << "Please click on the upper right coordinate of the region of intrest" << endl;
	//Wait until ROI variables are set
	while (!ROI_is_set) {
		//Copy frame to draw rectangle
		Mat copyFrame;
		Frame.copyTo(copyFrame);
		//Check if min coordinates of ROI are set
		if (ROI_x_min > 0 && ROI_y_min > 0 && mouse_x >= ROI_x_min && mouse_y >= ROI_y_min)
		{
			//Draw rectangle 
			cv::Rect rect = cv::Rect(ROI_x_min, ROI_y_min, mouse_x - ROI_x_min, mouse_y - ROI_y_min);
			rectangle(copyFrame, rect, Scalar(0, 0, 255), 2);
		}
		//show the frame with rect
		imshow("Select ROI", copyFrame);
		waitKey(10);
	}
	//Destroy window with first frame
	cv::destroyWindow("Select ROI");
	//Return rect with region of intrest
	return cv::Rect(ROI_x_min, ROI_y_min, ROI_x_max - ROI_x_min, ROI_y_max - ROI_y_min);
}

//Function that creates a window with trackbars
void createTrackbars() {
	//create window for trackbars
	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf(TrackbarName, "Threshold", threshold_value);
	sprintf(TrackbarName, "Erode", erode_value);
	sprintf(TrackbarName, "Dilate", dilate_value);
	sprintf(TrackbarName, "Min Area", min_area);
	sprintf(TrackbarName, "Y-line", y_line);
	//create trackbars  
	createTrackbar("Threshold", trackbarWindowName, &threshold_value, 100, on_trackbar);
	createTrackbar("Erode", trackbarWindowName, &erode_value, 100, on_trackbar);
	createTrackbar("Dilate", trackbarWindowName, &dilate_value, 100, on_trackbar);
	createTrackbar("Min Area", trackbarWindowName, &min_area, 5000, on_trackbar);
	createTrackbar("Y-line", trackbarWindowName, &y_line, y_max, on_trackbar);
}

void draw_cars(Mat differenceFiltered, Mat &next) {
	//Find contours in the image
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	//findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );// retrieves all contours
	findContours(differenceFiltered, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);// retrieves external contours
	for (int i = 0; i < contours.size(); i++)
	{
		cv::Rect foundRect = boundingRect(contours.at(i));
		if (foundRect.y > y_line && foundRect.area() > min_area)
		{
			rectangle(next, foundRect, Scalar(0, 255, 0), 2);
			int xpos = foundRect.x + foundRect.width / 2;
			int ypos = foundRect.y + foundRect.height / 2;
			putText(next, "Area = " + std::to_string(foundRect.area()), Point(xpos, ypos), 1, 1, Scalar(255, 0, 0), 1);
		}
	}
	//Draw y-line
	line(next, Point(0, y_line), Point(x_max, y_line), Scalar(0, 0, 255),1);
}

void track_cars(Mat previous, Mat &next){
	//Convert to gray images
	Mat previousGray, nextGray;
	cv::cvtColor(previous, previousGray, COLOR_BGR2GRAY);
	cv::cvtColor(next, nextGray, COLOR_BGR2GRAY);
	//Calculate difference
	Mat difference;
	cv::absdiff(previousGray, nextGray, difference);
	imshow("Difference", difference);
	//Threshold difference
	Mat differenceThreshold;
	cv::threshold(difference, differenceThreshold, threshold_value, 255, THRESH_BINARY);
	imshow("Difference Threshold", differenceThreshold);
	//Filter out noise
	Mat differenceFiltered;
	differenceThreshold.copyTo(differenceFiltered);
	//Dilate fills black spaces
	if (dilate_value > 0)
	{
		Mat dilate_settings = getStructuringElement(MORPH_RECT, Size(dilate_value, dilate_value));
		dilate(differenceFiltered, differenceFiltered, dilate_settings);
	}
	//Erode deletes pixels who are smaller than a given value
	if (erode_value > 0)
	{
		Mat erode_settings = getStructuringElement(MORPH_RECT, Size(erode_value, erode_value));
		erode(differenceFiltered, differenceFiltered, erode_settings);
	}
	//Show filtered
	imshow("Filtered", differenceFiltered);
	//Draw cars
	draw_cars(differenceFiltered, next);
}

int main() {
	//Create capture object to open the video(s)
	VideoCapture capture;
	//Open the needed video
	capture.open("video1.mp4");
	//Frames which we manipulate
	Mat currentFrame, previousFrame;
	//Get first frame
	capture.read(previousFrame);
	//Get region of intrest
	cv::Rect ROI = getROI(previousFrame);
	//Adjust to ROI
	previousFrame = previousFrame(ROI);
	//Set screen settings
	x_max = ROI.width;
	y_max = ROI.height;
	//create track bars
	createTrackbars();
	//Go over the video untill the last frame
	while (capture.get(CV_CAP_PROP_POS_FRAMES) < capture.get(CV_CAP_PROP_FRAME_COUNT)) {
		//Get next frame
		capture.read(currentFrame);
		//Get the ROI of frame
		currentFrame = currentFrame(ROI);
		//Copy currentFrame
		Mat currentFrameCopy;
		currentFrame.copyTo(currentFrameCopy);
		//Track cars
		track_cars(previousFrame, currentFrameCopy);
		//Show this frame
		imshow("Video", currentFrameCopy);
		//Show frame for a given time while waiting for keypress
		switch (waitKey(1000 / FRAMES_PER_SECOND))
		{
			//Stop program in case ESC is pressed
			case 27: return 0;
		}
		//The previous frame is now the next frame
		currentFrame.copyTo(previousFrame);
	}
	//Succeeded return
	return 0;
}