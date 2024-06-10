#pragma once
#include <cmath>
class VectorObject {
private:

public:
	double x;
	double y;
	double z;

	VectorObject();
	VectorObject(double x, double y, double z);

	VectorObject getUnitVector();
	double getMagnitude();
	VectorObject multipliedBy(double factor);
	VectorObject subtractedBy(VectorObject vector);

	//State change functions
	void addVectorObject(VectorObject change);
	void setMagnitude(double newMagnitude);
	void capMagnitude(double cappedMagnitude);
};