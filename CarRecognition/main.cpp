#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <windows.h>
#include "Car.h"

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
int y_line = 180;
vector<Scalar> allColors = {Scalar(255,248,240),Scalar(215,235,250),Scalar(255,255,0),Scalar(212,255,127),Scalar(255,255,240),Scalar(220,245,245),Scalar(196,228,255),Scalar(0,0,0),Scalar(205,235,255),Scalar(255,0,0),Scalar(226,43,138),Scalar(42,42,165),Scalar(135,184,222),Scalar(160,158,95),Scalar(0,255,127),Scalar(30,105,210),Scalar(80,127,255),Scalar(237,149,100),Scalar(220,248,255),Scalar(60,20,220),Scalar(255,255,0),Scalar(139,0,0),Scalar(139,139,0),Scalar(11,134,184),Scalar(169,169,169),Scalar(0,100,0),Scalar(169,169,169),Scalar(107,183,189),Scalar(139,0,139),Scalar(47,107,85),Scalar(0,140,255),Scalar(204,50,153),Scalar(0,0,139),Scalar(122,150,233),Scalar(143,188,143),Scalar(139,61,72),Scalar(79,79,47),Scalar(79,79,47),Scalar(209,206,0),Scalar(211,0,148),Scalar(147,20,255),Scalar(255,191,0),Scalar(105,105,105),Scalar(105,105,105),Scalar(255,144,30),Scalar(34,34,178),Scalar(240,250,255),Scalar(34,139,34),Scalar(255,0,255),Scalar(220,220,220),Scalar(255,248,248),Scalar(0,215,255),Scalar(32,165,218),Scalar(128,128,128),Scalar(0,128,0),Scalar(47,255,173),Scalar(128,128,128),Scalar(240,255,240),Scalar(180,105,255),Scalar(92,92,205),Scalar(130,0,75),Scalar(240,255,255),Scalar(140,230,240),Scalar(250,230,230),Scalar(245,240,255),Scalar(0,252,124),Scalar(205,250,255),Scalar(230,216,173),Scalar(128,128,240),Scalar(255,255,224),Scalar(210,250,250),Scalar(211,211,211),Scalar(144,238,144),Scalar(211,211,211),Scalar(193,182,255),Scalar(122,160,255),Scalar(170,178,32),Scalar(250,206,135),Scalar(153,136,119),Scalar(153,136,119),Scalar(222,196,176),Scalar(224,255,255),Scalar(0,255,0),Scalar(50,205,50),Scalar(230,240,250),Scalar(255,0,255),Scalar(0,0,128),Scalar(170,205,102),Scalar(205,0,0),Scalar(211,85,186),Scalar(219,112,147),Scalar(113,179,60),Scalar(238,104,123),Scalar(154,250,0),Scalar(204,209,72),Scalar(133,21,199),Scalar(112,25,25),Scalar(250,255,245),Scalar(225,228,255),Scalar(181,228,255),Scalar(173,222,255),Scalar(128,0,0),Scalar(230,245,253),Scalar(0,128,128),Scalar(35,142,107),Scalar(0,165,255),Scalar(0,69,255),Scalar(214,112,218),Scalar(170,232,238),Scalar(152,251,152),Scalar(238,238,175),Scalar(147,112,219),Scalar(213,239,255),Scalar(185,218,255),Scalar(63,133,205),Scalar(203,192,255),Scalar(221,160,221),Scalar(230,224,176),Scalar(128,0,128),Scalar(0,0,255),Scalar(143,143,188),Scalar(225,105,65),Scalar(19,69,139),Scalar(114,128,250),Scalar(96,164,244),Scalar(87,139,46),Scalar(238,245,255),Scalar(45,82,160),Scalar(192,192,192),Scalar(235,206,135),Scalar(205,90,106),Scalar(144,128,112),Scalar(144,128,112),Scalar(250,250,255),Scalar(127,255,0),Scalar(180,130,70),Scalar(140,180,210),Scalar(128,128,0),Scalar(216,191,216),Scalar(71,99,255),Scalar(208,224,64),Scalar(238,130,238),Scalar(179,222,245),Scalar(255,255,255),Scalar(245,245,245),Scalar(0,255,255),Scalar(50,205,154) };

//List of current found cars
vector<Car> currentCars;

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
		if (ROI_x_min > 0 && ROI_y_min > 0 && mouse_x > ROI_x_min && mouse_y > ROI_y_min)
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
	//A vector storing the new current cars
	vector<Car> newCurrentCars;
	//Find contours in the image
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(differenceFiltered, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);// retrieves external contours
	for (int i = 0; i < contours.size(); i++)
	{
		cv::Rect foundRect = boundingRect(contours.at(i));
		if (foundRect.y > y_line && foundRect.area() > min_area)
		{
			int xpos = foundRect.x + foundRect.width / 2;
			int ypos = foundRect.y + foundRect.height / 2;
			//putText(next, "Area = " + std::to_string(foundRect.area()), Point(xpos, ypos), 1, 1, Scalar(255, 0, 0), 1);
			//Create a new car with the given coordinates
			Car newCar = Car(foundRect.x, foundRect.y);
			//Search in the list of cars of the previous iteration
			//to the nearest car
			try 
			{
				Car nearestCar = newCar.findNearest(currentCars);
				cout << "Distance = " + to_string(newCar.distance(nearestCar)) << endl;
				if (newCar.distance(nearestCar) <= 5)
				{
					//If nearest car is found and not to far
					//give the new car the same color
					newCar.setColor(nearestCar.getColor());
					//Delete car from current car list
					//So the color cannot be used again
					currentCars.erase(find(currentCars.begin(), currentCars.end(), nearestCar));
				}
				else {
					//if nearest car is to far
					//we take a color from the all colors list
					newCar.setColor(allColors.back());
					//remove the color from the vector
					//so it's not used again
					allColors.pop_back();
				}
				
			}
			catch (invalid_argument e)
			{
				//if nearest car is not found 
				//we take a color from the all colors list
				newCar.setColor(allColors.back());
				//remove the color from the vector
				//so it's not used again
				allColors.pop_back();
			}
			//add the car to the newCurrent cars list
			newCurrentCars.push_back(newCar);
			//Draw rectangle arround the car with the cars color
			rectangle(next, foundRect, newCar.getColor(), 2);
		}
	}
	//add the dissapearing car colors back to the color list
	for (int i = 0; i < currentCars.size(); i++) {
		allColors.insert(allColors.begin(),currentCars.at(i).getColor());
	}
	//Make the currentCarList equal to the newCurrentCar vector
	currentCars = newCurrentCars;
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
	//Erode deletes black spaces who are smaller than a given value
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