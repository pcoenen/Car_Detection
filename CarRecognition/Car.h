#pragma once
#include <opencv2/opencv.hpp>
class Car
{
private:
	int x, y;
	cv::Scalar color;
public:
	Car(int x, int y);
	~Car();
	int getX();
	int getY();
	cv::Scalar getColor();
	Car findNearest(std::vector<Car> list);
	double distance(int x, int y);
	double distance(Car car);
	void setColor(cv::Scalar color);
};

