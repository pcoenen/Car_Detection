#include "Car.h"

Car::Car(int x, int y)
{
	this->x = x;
	this->y = y;
}

//Destuctor
Car::~Car() {};

//Returns the x-coordinate of this car
int Car::getX()
{
	return this->x;
}

//Returns the y-coordinate of this car
int Car::getY()
{
	return this->y;
}

//Returns the color of this car
cv::Scalar Car::getColor()
{
	return this->color;
}

//Sets the color of this car
void Car::setColor(cv::Scalar color)
{
	this->color = (color);
}

//This function searches for the car that is the closest to this car
Car Car::findNearest(std::vector<Car> list)
{
	if (list.size() == 0) {
		throw std::invalid_argument("empty list");
	}
	double minDistance = distance(list.at(0).getX(), list.at(0).getY());
	Car minCar = list.at(0);
	for (int i = 1; i < list.size(); i++) {
		Car currentCar = list.at(i);
		double currentDistance = distance(currentCar);
		if (currentDistance < minDistance)
		{
			minDistance = currentDistance;
			minCar = currentCar;
		}
	}
	return minCar;
}

//Calculates the distance from this car to given coordinates
double Car::distance(int x, int y)
{
	return sqrt(((this->y)-y) ^ 2 + ((this->x)-x) ^ 2);
}

//Calculates the distance from this car to a given car
double Car::distance(Car car)
{
	return distance(car.getX, car.getY);
}
