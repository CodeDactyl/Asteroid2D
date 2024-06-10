#pragma once
#include "Structures.h"
#include <vector>
#include "VectorObject.h"

static const double DEFAULT_SCALE = 0.1;
static const double DEFAULT_MAX_VELOCITY = 0.5;
static const double DEFAULT_ACCELERATION = 0.4;
static const double DEFAULT_RATE_DAMPENING = 0.3;

class Object {
private:

public:
	//Physics Definition
	double max_velocity = DEFAULT_MAX_VELOCITY;
	double max_acceleration = DEFAULT_ACCELERATION;
	double scaleX = DEFAULT_SCALE;
	double scaleY = DEFAULT_SCALE;
	double scaleZ = DEFAULT_SCALE;
	double rotation = 0.0;
	double rotationVelocity = 0.0;
	double hitRadius = 0.0;
	VectorObject position;
	VectorObject velocity;
	VectorObject acceleration;

	//Visual Definition
	bool visible = true;
	int glMode = 0;
	std::vector<VectorObject> shape;
	double red = 0.0;
	double green = 0.0;
	double blue = 0.0;

	//Misc Definition
	double age = 0.0;
	bool marked = false;

	Object();

	void accelerateObject(VectorObject jerk, double deltaTime); //Yes that is the term for change in acceleration
	void dampenObject(double deltaTime);
	void updatePhysics(double deltaTime); 
};