#include "Object.h"

Object::Object() {
}

//Accelerate object
void Object::accelerateObject(VectorObject jerk, double deltaTime) {
	acceleration = jerk.multipliedBy(DEFAULT_ACCELERATION);
	acceleration.capMagnitude(max_acceleration);
}

//"Magic Space Friction" function
void Object::dampenObject(double deltaTime) {
	acceleration = VectorObject(0.0, 0.0, 0.0);
	velocity.addVectorObject(velocity.getUnitVector().multipliedBy(-DEFAULT_RATE_DAMPENING * deltaTime));
}

//Cascading Update
void Object::updatePhysics(double deltaTime) {
	age += deltaTime;

	velocity.addVectorObject(acceleration.multipliedBy(deltaTime));
	velocity.capMagnitude(max_velocity);
	position.addVectorObject(velocity.multipliedBy(deltaTime));

	rotation = rotation + deltaTime * rotationVelocity;
	if (rotation < 0.0) {
		rotation = 360.0 + rotation;
	}
	if (rotation >= 360.0) {
		rotation = std::fmod(rotation, 360.0);
	}
}