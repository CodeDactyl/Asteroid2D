#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <iomanip>
#include <sstream>
#include <random>
#include "../vs/VectorObject.h"
#include "../vs/Object.h"

#if _WIN32
# include <windows.h>
#endif
#if __APPLE__
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# include <GLUT/glut.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif

//Constants
const bool DEBUG_MODE = 1;
const double WALL_WIDTH = 0.05;
const double WALL_PROXIMITY = 0.3;
const double PI = 3.14159265358979323846;
const double SIZE_BULLET = 3.0;
const double RATE_BULLET = 5;
const double RATE_MISSILE = 0.5;
const double WAVE_TIME = 10.0;
const double L0 = 0.0;
const double L1 = 0.1;
const double L2 = 0.2;
const std::string MESSAGE_START = "Press Any Key To Start";
const std::string MESSAGE_RESUME = "Game Over. Press any key to play again";

//Variables
int window_height;
int window_width;
int window_param;
int score = 0;
bool keys[256];
bool mouse_held_left = false;
bool mouse_held_right = false;
bool fullscreen = true;
bool weaponAlt = false;
double weaponCooldown = 0.0;
double waveCooldown = 0.0;
int waveCount = 0;
double gameTime = 0.0;
bool gameRunning = false;
bool firstRound = true;
bool resumeGame = false;

//GameObjects
Object spaceShip;
Object gunBarrel;
std::vector<Object> asteroids;
std::vector<int> asteroidHealth;
std::vector<Object> bullets;
std::vector<Object> missiles;
std::vector<Object> particles;

//Delta Time Calucations
int elapsedTime = 0;
int pFrameTime = 0;
double deltaTime = 0.0;

double getVisibilityRadius() {
	double visibilityRadius;
	if (window_height >= window_width) {
		visibilityRadius = sqrt(2) * ((double)window_height / (double)window_width);
	} else {
		visibilityRadius = sqrt(2) * ((double)window_width / (double)window_height);
	}
	return visibilityRadius;
}

double getRandomDouble(double min, double max) {
	std::random_device rd;
	std::default_random_engine eng(rd());
	std::uniform_real_distribution<float> distr(min, max);
	return distr(eng);
}

double bearingToRadian(double bearing) {
	double degree = 90.0 - bearing;
	if (degree >= 360.0) {
		degree -= 360.0;
	}
	if (degree < 0.0) {
		degree += 360.0;
	}
	return degree * (PI / 180.0);
}

void particleCloud(VectorObject location){
	int particleCount = 10;
	for (int i = 0; i < particleCount; i++){
		Object particle;

		particle.shape.push_back(VectorObject(0.0, 0.0, 0.4));
		particle.glMode = GL_POINTS;
		particle.red = 1.0;
		particle.green = 1.0;
		particle.blue = 0.0;

		particle.scaleX = 0.1;
		particle.scaleY = 0.1;
		particle.scaleZ = 0.1;
		particle.position = location;
		double direction = getRandomDouble(0.0, 359.0);
		double radian = bearingToRadian(direction);
		double particleSpeed = getRandomDouble(0.08, 0.1);
		VectorObject velocity = VectorObject(cos(radian), sin(radian), 0.0);
		particle.velocity = velocity.multipliedBy(particleSpeed);

		particles.push_back(particle);
	}
}

//Draw Object
void drawObject(Object object) {
	if (object.visible) {
		glPushMatrix();
		glTranslated(object.position.x, object.position.y, object.position.z);
		glRotated(object.rotation, 0.0, 0.0, -1.0);
		glScaled(object.scaleX, object.scaleY, object.scaleZ);
		glColor3d(object.red, object.green, object.blue);
		glBegin(object.glMode);
		for (int i = 0; i < object.shape.size(); i++) {
			glVertex3d(object.shape.at(i).x, object.shape.at(i).y, object.shape.at(i).z);
		}
		glEnd();
		glPopMatrix();
	}
}

void spawnAsteroid() {
	Object asteroid;
	asteroid.glMode = GL_LINE_LOOP;
	asteroid.red = 1.0;
	asteroid.green = 1.0;
	asteroid.blue = 1.0;
	asteroid.scaleX = 0.1;
	asteroid.scaleY = 0.1;
	asteroid.scaleZ = 0.1;
	asteroid.rotationVelocity = getRandomDouble(-360.0, 360.0);

	/*
	* Asteroid start position Algorithm
	* Find a random point on a circle by angle from 0-359
	* Convert angle to a unit vector
	* Multiply vector by visibility radius root 2 * highest aspect ratio
	*/
	double bearingOrigin = getRandomDouble(0.0, 359.0);
	double radian = bearingToRadian(bearingOrigin);
	VectorObject asteroidDirection = VectorObject(cos(radian), sin(radian), 0.0);
	asteroid.position = asteroidDirection.multipliedBy(getVisibilityRadius());

	/*
	* Asteroid velocity/direction Algorithm
	* Direction vector is PlayerPos - AstPos
	*/
	double astSpeed = getRandomDouble(0.01, 0.3);
	//VectorObject astDirection = spaceShip.position.subtractedBy(asteroid.position);
	VectorObject astDirection(spaceShip.position.x - asteroid.position.x, spaceShip.position.y - asteroid.position.y, 0.0);
	asteroid.velocity = astDirection.getUnitVector().multipliedBy(astSpeed);

	/*
	* Random shape Algorithm
	* generate random number of edges
	* 360/num = degrees between each edge from center
	* convert each angle to a unit vector of origin (generates something valguely circular)
	* for each unit vector multiply by a random double
	* for each edge vector, translate it in a randomish direction slightly
	*/
	int health = 1;
	int edgeCount = (int)getRandomDouble(5.0, 10.0);
	double edgeAngleDiff = 360.0 / (double)edgeCount;
	double hitRadius = 0.0;

	for (int i = 0; i < edgeCount; i++) {
		double angle = i * edgeAngleDiff;
		double randomSize = getRandomDouble(0.5, 0.9);
		VectorObject unitVector = VectorObject(cos(bearingToRadian(angle)), sin(bearingToRadian(angle)), 0.0);
		VectorObject edge = unitVector.multipliedBy(randomSize);
		if (randomSize > 0.88){
			health = 2;
			asteroid.red = 1.0;
			asteroid.blue = 0.0;
			asteroid.green = 0.0;
		}
		if (edge.getMagnitude() > hitRadius) {
			hitRadius = edge.getMagnitude();
		}
		edge.z = L2;
		asteroid.shape.push_back(edge);
	}
	asteroid.hitRadius = asteroid.scaleX * hitRadius;

	//Add asteroid
	asteroidHealth.push_back(health);
	asteroids.push_back(asteroid);
}

void weaponAction(double time) {
	if (weaponAlt == false) {
		if (weaponCooldown <= 0.0) {
			double gunBarrelLength = 0.01;

			//Generate Bullet
			Object bullet;
			double radian = bearingToRadian(spaceShip.rotation);
			bullet.glMode = GL_POINTS;
			bullet.shape.push_back(VectorObject(0.0, 0.0, L2));
			bullet.rotation = spaceShip.rotation;

			//Knightmare bullet stuff
			double radian2 = bearingToRadian(spaceShip.rotation);
			VectorObject rotationVector(cos(radian2), sin(radian2), 0.0);
			VectorObject relativeVector = rotationVector.multipliedBy(spaceShip.hitRadius + gunBarrelLength);
			bullet.position = VectorObject(relativeVector.x + spaceShip.position.x, relativeVector.y + spaceShip.position.y, spaceShip.position.z);

			bullet.max_velocity = 1.0;
			bullet.velocity = VectorObject(cos(radian), sin(radian), 0.0);
			bullet.velocity.setMagnitude(1.0);
			bullet.scaleX = 0.1;
			bullet.scaleY = 0.1;
			bullet.scaleZ = 0.1;
			bullet.red = 1.0;
			bullet.green = 1.0;
			bullet.blue = 1.0;
			bullets.push_back(bullet);

			//Reset weapon cooldown
			weaponCooldown = 1.0 / RATE_BULLET;
		} else {
			weaponCooldown -= time;
		}
	} else {
		if (weaponCooldown <= 0.0) {
			//TODO Generate Missile

			weaponCooldown = 1.0 / RATE_MISSILE;
		} else {
			weaponCooldown -= time;
		}
	}
}

void specialAction() {
	if (DEBUG_MODE){
		spawnAsteroid();
	}
}

void weaponSwitch() {
	weaponAlt = !weaponAlt;
}

//Game Object Setup
void gameInit() {
	//Setup spaceShip Object
	spaceShip.glMode = 2;
	spaceShip.shape.push_back(VectorObject(0.0, 0.4, 0.5));
	spaceShip.shape.push_back(VectorObject(0.1, -0.1, 0.5));
	spaceShip.shape.push_back(VectorObject(0.2, 0.0, 0.5));
	spaceShip.shape.push_back(VectorObject(0.3, -0.3, 0.5));
	spaceShip.shape.push_back(VectorObject(0.0, -0.2, 0.5));
	spaceShip.shape.push_back(VectorObject(-0.3, -0.3, 0.5));
	spaceShip.shape.push_back(VectorObject(-0.2, 0.0, 0.5));
	spaceShip.shape.push_back(VectorObject(-0.1, -0.1, 0.5));
	spaceShip.scaleX = 0.15;
	spaceShip.scaleY = 0.15;
	spaceShip.scaleZ = 0.15;
	spaceShip.red = 0.5;
	spaceShip.green = 0.5;
	spaceShip.blue = 0.5;
	spaceShip.hitRadius = 0.075;
	spaceShip.position = VectorObject(0.0, 0.0, L2);
	spaceShip.velocity = VectorObject(0.0, 0.0, 0.0);
	spaceShip.acceleration = VectorObject(0.0, 0.0, 0.0);

	gunBarrel = spaceShip;
	gunBarrel.glMode = GL_QUADS;
	gunBarrel.shape.clear();
	gunBarrel.shape.push_back(VectorObject(0.06, 0.6, 0.4));
	gunBarrel.shape.push_back(VectorObject(0.06, 0.4, 0.4));
	gunBarrel.shape.push_back(VectorObject(-0.06, 0.4, 0.4));
	gunBarrel.shape.push_back(VectorObject(-0.06, 0.6, 0.4));
	
}

void drawBoundry(VectorObject playerLocation) {

	//N
	if (playerLocation.y > 1 - WALL_WIDTH - WALL_PROXIMITY) {
		glColor3d(1.0, 0.290, 0.071);
	} else {
		glColor3d(0.153, 0.290, 0.871);
	}
	glBegin(GL_QUADS);
	glVertex3d(1, 1, L1);
	glVertex3d(1, 1 - WALL_WIDTH, L1);
	glVertex3d(-1, 1 - WALL_WIDTH, L1);
	glVertex3d(-1, 1, L1);
	glEnd();

	//S
	if (playerLocation.y < -1 + WALL_WIDTH + WALL_PROXIMITY) {
		glColor3d(1.0, 0.290, 0.071);
	} else {
		glColor3d(0.153, 0.290, 0.871);
	}
	glBegin(GL_QUADS);
	glVertex3d(1, -1 + WALL_WIDTH, L1);
	glVertex3d(1, -1, L1);
	glVertex3d(-1, -1, L1);
	glVertex3d(-1, -1 + WALL_WIDTH, L1);
	glEnd();

	//E
	if (playerLocation.x < -1 + WALL_WIDTH + WALL_PROXIMITY) {
		glColor3d(1.0, 0.290, 0.071);
	} else {
		glColor3d(0.153, 0.290, 0.871);
	}
	glBegin(GL_QUADS);
	glVertex3d(-1 + WALL_WIDTH, 1, L1);
	glVertex3d(-1 + WALL_WIDTH, -1, L1);
	glVertex3d(-1, -1, L1);
	glVertex3d(-1, 1, L1);
	glEnd();

	//W
	if (playerLocation.x > 1 - WALL_WIDTH - WALL_PROXIMITY) {
		glColor3d(1.0, 0.290, 0.071);
	} else {
		glColor3d(0.153, 0.290, 0.871);
	}
	glBegin(GL_QUADS);
	glVertex3d(1, 1, L1);
	glVertex3d(1, -1, L1);
	glVertex3d(1 - WALL_WIDTH, -1, L1);
	glVertex3d(1 - WALL_WIDTH, 1, L1);
	glEnd();
}

bool boundryCheck(Object object) {
	bool collision = false;
	if (object.position.y > 1 - (WALL_WIDTH + object.hitRadius)) {
		collision = true;
	}

	if (object.position.y < -1 + (WALL_WIDTH + object.hitRadius)) {
		collision = true;
	}

	if (object.position.x < -1 + (WALL_WIDTH + object.hitRadius)) {
		collision = true;
	}

	if (object.position.x > 1 - (WALL_WIDTH + object.hitRadius)) {
		collision = true;
	}
	return collision;
}

// Display callback
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPointSize(SIZE_BULLET);

	// Background
	glColor3d(0.122, 0.0118, 0.133);
	glBegin(GL_POLYGON);
	glVertex3d(1, 1, L0);
	glVertex3d(1, -1, L0);
	glVertex3d(-1, -1, L0);
	glVertex3d(-1, 1, L0);
	glEnd();

	drawBoundry(spaceShip.position);
	drawObject(gunBarrel);
	drawObject(spaceShip);

	for (int i = 0; i < asteroids.size(); i++) {
		drawObject(asteroids.at(i));
	}

	for (int i = 0; i < bullets.size(); i++) {
		drawObject(bullets.at(i));
	}

	for (int i = 0; i < missiles.size(); i++) {
		drawObject(missiles.at(i));
	}

	for (int i = 0; i < particles.size(); i++) {
		drawObject(particles.at(i));
	}

	//Score Print
	glColor3d(0.2, 0.5, 0.2);
	int widthCounter = 0;
	double offset = 0.0;
	std::string text = "Score: " + std::to_string(score);
	for (int i = 0; i < text.length(); i++) {
		widthCounter += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
	}
	offset = (double)widthCounter / (double)window_param;
	glRasterPos3d(-1 + offset, 1-0.12, 0.5);
	for (int i = 0; i < text.length(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
	}

	//Time Print
	widthCounter = 0;
	offset = 0.0;
	std::stringstream stringStream;
	stringStream << std::fixed << std::setprecision(3) << gameTime;
	std::string time = "Time: " + stringStream.str();
	for (int i = 0; i < time.length(); i++) {
		widthCounter += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, time[i]);
	}
	offset = (double)widthCounter / (double)window_param;
	glRasterPos3d(1.0 - (3*offset), 1-0.12, 0.5);
	for (int i = 0; i < time.length(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, time[i]);
	}

	//Start Message
	if (gameRunning == false && resumeGame == false && firstRound == true) {
		int widthCounter = 0;
		double offset = 0.0;

		for (int i = 0; i < MESSAGE_START.length(); i++) {
			widthCounter += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, MESSAGE_START[i]);
		}
		offset = (double)widthCounter / (double)window_param;
		glRasterPos3d(-offset, 0.1, 0.5);
		for (int i = 0; i < MESSAGE_START.length(); i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, MESSAGE_START[i]);
		}
	}

	//End message
	if (gameRunning == false && resumeGame == false && firstRound == false) {
		int widthCounter = 0;
		double offset = 0.0;

		for (int i = 0; i < MESSAGE_RESUME.length(); i++) {
			widthCounter += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, MESSAGE_RESUME[i]);
		}

		offset = (double)widthCounter / (double)window_param;
		glRasterPos3d(-offset, 0.1, 0.5);
		for (int i = 0; i < MESSAGE_RESUME.length(); i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, MESSAGE_RESUME[i]);
		}
	}

	int err;
	while ((err = glGetError()) != GL_NO_ERROR)
		printf("display: %s\n", gluErrorString(err));

	glutSwapBuffers();
}

void gmGame(double deltaTime) {

	//Reset Game Function
	if (resumeGame) {
		resumeGame = false;
		gameRunning = true;
		asteroids.clear();
		bullets.clear();
		missiles.clear();
		particles.clear();
		waveCount = 0;
		waveCooldown = 0.0;
		score = 0;
		gameTime = 0.0;
		spaceShip.rotation = 0.0;
		spaceShip.position.x = 0.0;
		spaceShip.position.y = 0.0;
		spaceShip.velocity.x = 0.0;
		spaceShip.velocity.y = 0.0;
	}

	if (gameRunning) {
		gameTime += deltaTime;

		//Bullet Cleanup
		for (int i = 0; i < bullets.size(); i++) {
			if (bullets.at(i).age > 3.0 || boundryCheck(bullets.at(i))) {
				bullets.erase(bullets.begin() + i);
			}
		}

		//Asteroids wall bouncing
		for (int i = 0; i < asteroids.size(); i++) {
			double minDist = WALL_WIDTH + asteroids.at(i).hitRadius;
			double x = asteroids.at(i).position.x;
			double y = asteroids.at(i).position.y;

			if ((asteroids.at(i).velocity.y > 0.0) && (asteroids.at(i).position.y > 1 - minDist)) {
				asteroids.at(i).position.y = 1 - minDist;
				asteroids.at(i).velocity.y = -asteroids.at(i).velocity.y;
			}

			if ((asteroids.at(i).velocity.y < 0.0) && (asteroids.at(i).position.y < -1 + minDist)) {
				asteroids.at(i).position.y = -1 + minDist;
				asteroids.at(i).velocity.y = -asteroids.at(i).velocity.y;
			}

			if ((asteroids.at(i).velocity.x < 0.0) && (asteroids.at(i).position.x < -1 + minDist)) {
				asteroids.at(i).position.x = -1 + minDist;
				asteroids.at(i).velocity.x = -asteroids.at(i).velocity.x;
			}

			if ((asteroids.at(i).velocity.x > 0.0) && (asteroids.at(i).position.x > 1 - minDist)) {
				asteroids.at(i).position.x = 1 - minDist;
				asteroids.at(i).velocity.x = -asteroids.at(i).velocity.x;
			}
		}

		//Bullet DMG
		std::vector<int> bulletDelete;
		std::vector<int> asteroidDelete;
		for (int i = 0; i < bullets.size(); i++) {
			for (int j = 0; j < asteroids.size(); j++) {
				double minDist = bullets.at(i).hitRadius + asteroids.at(j).hitRadius;
				double xDist = bullets.at(i).position.x - asteroids.at(j).position.x;
				double yDist = bullets.at(i).position.y - asteroids.at(j).position.y;
				double trueDist = sqrt((2 * xDist * xDist) + (2 * yDist * yDist));

				if (minDist > trueDist) {
					score += 100;
					bullets.at(i).marked = true;
					asteroidHealth.at(j) -= 1; 
					if (asteroidHealth.at(j) <= 0){
						particleCloud(asteroids.at(j).position);
						asteroids.at(j).marked = true;
					} else {
						asteroids.at(j).green = 1.0;
						asteroids.at(j).blue = 1.0;
					}
				}
			}
		}

		//HELLA JANK Object list replacement to avoid issues with deleting a counting object
		std::vector<Object> newBullet;
		for (int i = 0; i < bullets.size(); i++) {
			if (bullets.at(i).marked == false) {
				newBullet.push_back(bullets.at(i));
			}
		}
		bullets = newBullet;

		std::vector<Object> newParticle;
		for (int i = 0; i < particles.size(); i++) {
			if (particles.at(i).age <= 1.75) {
				newParticle.push_back(particles.at(i));
			}
		}
		particles = newParticle;

		std::vector<Object> newAsteroid;
		std::vector<int> newAsteroidHealth;
		for (int i = 0; i < asteroids.size(); i++) {
			if (asteroids.at(i).marked == false) {
				newAsteroidHealth.push_back(asteroidHealth.at(i));
				newAsteroid.push_back(asteroids.at(i));
			}
		}
		asteroids = newAsteroid;
		asteroidHealth = newAsteroidHealth;

		//Asteroid collision function
		for (int i = 0; i < asteroids.size(); i++) {
			for (int j = 0; j < asteroids.size(); j++) {
				if (i != j) {
					double minDist = asteroids.at(i).hitRadius + asteroids.at(j).hitRadius;
					double xDist = asteroids.at(i).position.x - asteroids.at(j).position.x;
					double yDist = asteroids.at(i).position.y - asteroids.at(j).position.y;
					double trueDist = sqrt((2 * xDist * xDist) + (2 * yDist * yDist));
					if (minDist > trueDist) {
						double xCenter = (asteroids.at(i).position.x + asteroids.at(j).position.x) / 2.0;
						double yCenter = (asteroids.at(i).position.y + asteroids.at(j).position.y) / 2.0;
						VectorObject collisionCenter(xCenter, yCenter, L2);

						//Normal vector to collision
						VectorObject iVector(asteroids.at(i).position.x - xCenter, asteroids.at(i).position.y - yCenter, 0.0);
						VectorObject jVector(asteroids.at(j).position.x - xCenter, asteroids.at(j).position.y - yCenter, 0.0);
						VectorObject iUnitVector = iVector.getUnitVector();
						VectorObject jUnitVector = jVector.getUnitVector();

						asteroids.at(i).velocity = iUnitVector.multipliedBy(asteroids.at(i).velocity.getMagnitude());
						asteroids.at(j).velocity = jUnitVector.multipliedBy(asteroids.at(j).velocity.getMagnitude());
					}
				}
			}
		}

		//Wave spawner
		if (waveCooldown < 0.0) {
			waveCount += 1;
			waveCooldown = WAVE_TIME;
			for (int i = 0; i < waveCount; i++) {
				spawnAsteroid();
			}
		} else {
			waveCooldown -= deltaTime;
		}

		//Player Asteroid collision check
		bool shipCollision = false;
		if (boundryCheck(spaceShip)) {
			shipCollision = true;
		}
		for (int i = 0; i < asteroids.size(); i++) {
			double minDist = spaceShip.hitRadius + asteroids.at(i).hitRadius;
			double xDist = spaceShip.position.x - asteroids.at(i).position.x;
			double yDist = spaceShip.position.y - asteroids.at(i).position.y;
			double trueDist = sqrt((2 * xDist * xDist) + (2 * yDist * yDist));
			if (minDist > trueDist) {
				shipCollision = true;
			}
		}
		if (shipCollision) {
			gameRunning = false;
			firstRound = false;
		}
	}
}

void processPhysics(double deltaTime) {
	if (gameRunning) {
		spaceShip.updatePhysics(deltaTime);
		gunBarrel.position = spaceShip.position;
		gunBarrel.rotation = spaceShip.rotation;

		for (int i = 0; i < asteroids.size(); i++) {
			asteroids.at(i).updatePhysics(deltaTime);
		}

		for (int i = 0; i < bullets.size(); i++) {
			bullets.at(i).updatePhysics(deltaTime);
		}

		for (int i = 0; i < missiles.size(); i++) {
			missiles.at(i).updatePhysics(deltaTime);
		}

		for (int i = 0; i < particles.size(); i++) {
			particles.at(i).updatePhysics(deltaTime);
		}
	}
}

void toggleFullscreen() {
	if (fullscreen == false) {
		glutFullScreen();
	} else {
		glutPositionWindow(0, 0);
		glutReshapeWindow(1200, 800);
	}
	fullscreen = !fullscreen;
}

//Read from key list and act on them
void computeControls(double deltaTime) {
	if (gameRunning == true) {

		if (keys[27] == true || keys[113] == true) {
			keys[27] = false;
			keys[113] = false;
			exit(EXIT_SUCCESS);
		}

		if (keys['r'] == true) {
			keys['r'] = false;
			toggleFullscreen();
		}

		if (keys['a'] != keys['d']) {
			if (keys['a']) {
				spaceShip.rotationVelocity = -120.0;
			} else {
				spaceShip.rotationVelocity = 120.0;
			}
		} else {
			spaceShip.rotationVelocity = 0.0;
		}

		if (keys['w'] == true) {
			double radian = bearingToRadian(spaceShip.rotation);
			spaceShip.accelerateObject(VectorObject(cos(radian), sin(radian), 0.0), deltaTime);
		} else {
			spaceShip.dampenObject(deltaTime);
		}

		if (keys['z']) {
			keys['z'] = false;
			weaponSwitch();
		}

		if (keys['x']) {
			keys['x'] = false;
			specialAction();
		}

		if (mouse_held_left) {
			weaponAction(deltaTime);
		}
	}
}

//Keyboard down callback
void keyDown(unsigned char key, int x, int y) {
	if (gameRunning == false) {
		resumeGame = true;
	}

	keys[key] = true;
}

//Keyboard up callback
void keyUp(unsigned char key, int x, int y) {
	keys[key] = false;
}

//Mouse callback
void mouse(int button, int state, int x, int y) {
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			mouse_held_left = true;
		} else {
			mouse_held_left = false;
		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN) {
			mouse_held_right = true;
		} else {
			mouse_held_right = false;
		}
		break;
	default:
		break;
	}
}

//Gameloop
void on_idle() {
	//Delta Time
	elapsedTime = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = (elapsedTime - pFrameTime) / 1000.0;
	pFrameTime = elapsedTime;

	//Update
	computeControls(deltaTime);
	processPhysics(deltaTime);
	gmGame(deltaTime);

	glutPostRedisplay();
}

void reshape(int w, int h) {
	window_width = w;
	window_height = h;
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h) {
		window_param = w;
		float aspect = (float)h / (float)w;
		glOrtho(-1.0, 1.0, -1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
	} else {
		window_param = h;
		float aspect = (float)w / (float)h;
		glOrtho(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0, -1.0, 1.0);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//Once off initial GL calls
void glInit() {
	//Visual Init
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Asteroids");
	glutFullScreen();
	glMatrixMode(GL_PROJECTION);
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

	/* Display and keyboard callbacks */
	glutIgnoreKeyRepeat(1);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutMouseFunc(mouse);
	glutIdleFunc(on_idle);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glInit();
	gameInit();

	//Begin
	glutMainLoop();
	return EXIT_SUCCESS;
}
