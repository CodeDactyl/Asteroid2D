#include "VectorObject.h"

VectorObject::VectorObject() {
	x = 0.0;
	y = 0.0;
	z = 0.0;
}

VectorObject::VectorObject(double x, double y, double z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

VectorObject VectorObject::getUnitVector() {
	double magnitude = std::sqrt((x * x) + (y * y) + (z * z));
	if (magnitude == 0){
		return VectorObject(0.0, 0.0, 0.0);
	}
	return VectorObject(x/magnitude, y/magnitude, z/magnitude);
}

double VectorObject::getMagnitude() {
	return std::sqrt((x * x) + (y * y) + (z * z));
}

VectorObject VectorObject::multipliedBy(double factor) {
	return VectorObject(x*factor, y*factor, z*factor);
}

VectorObject VectorObject::subtractedBy(VectorObject vector) {
	return VectorObject(x-vector.x, y-vector.y, z-vector.z);
}

void VectorObject::addVectorObject(VectorObject change) {
	x += change.x;
	y += change.y;
	z += change.z;
}

void VectorObject::setMagnitude(double newMagnitude) {
	VectorObject tempVectorObject = getUnitVector();
	x = tempVectorObject.x * newMagnitude;
	y = tempVectorObject.y * newMagnitude;
	z = tempVectorObject.z * newMagnitude;
}

void VectorObject::capMagnitude(double cappedMagnitude) {
	if (getMagnitude() > cappedMagnitude) {
		setMagnitude(cappedMagnitude);
	}
}