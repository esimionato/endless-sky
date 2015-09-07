/* Engine.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef ENGINE_H_
#define ENGINE_H_

#include "AI.h"
#include "AsteroidField.h"
#include "DrawList.h"
#include "EscortDisplay.h"
#include "Information.h"
#include "Point.h"
#include "Projectile.h"
#include "Radar.h"
#include "Ship.h"
#include "ShipEvent.h"

#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <thread>
#include <vector>

class Government;
class Outfit;
class PlayerInfo;



// Class representing the game engine: its job is to track all of the objects in
// the game, and to move them, step by step. All the motion and collision
// calculations are handled in a separate thread so that the graphics thread is
// free to just work on drawing things; this means that the drawn state of the
// game is always one step (1/60 second) behind what is being calculated. This
// lag is too small to be detectable and means that the game can better handle
// situations where there are many objects on screen at once.
class Engine {
public:
	Engine(PlayerInfo &player);
	~Engine();
	
	// Place all the player's ships, and "enter" the system the player is in.
	void Place();
	
	// Wait for the previous calculations (if any) to be done.
	void Wait();
	// Perform all the work that can only be done while the calculation thread
	// is paused (for thread safety reasons).
	void Step(bool isActive);
	// Begin the next step of calculations.
	void Go();
	
	// Get any special events that happened in this step.
	const std::list<ShipEvent> &Events() const;
	
	// Draw a frame.
	void Draw() const;
	
	// Select the object the player clicked on.
	void Click(const Point &point);
	
	
private:
	void EnterSystem();
	
	void ThreadEntryPoint();
	void CalculateStep();
	
	void DoGrudge(const std::shared_ptr<Ship> &target, const Government *attacker);
	
	
private:
	class Target {
	public:
		Point center;
		Angle angle;
		double radius;
		int type;
	};
	
	class Status {
	public:
		Status(const Point &position, double shields, double hull, double radius, bool isEnemy);
		
		Point position;
		double shields;
		double hull;
		double radius;
		bool isEnemy;
	};
	
	
private:
	PlayerInfo &player;
	
	AI ai;
	
	std::thread calcThread;
	std::condition_variable condition;
	std::mutex swapMutex;
	
	bool calcTickTock;
	bool drawTickTock;
	bool terminate;
	bool wasActive = false;
	DrawList draw[2];
	Radar radar[2];
	// Viewport position and velocity.
	Point position;
	Point velocity;
	// Other information to display.
	mutable Information info;
	std::vector<Target> targets;
	EscortDisplay escorts;
	std::vector<Status> statuses;
	std::vector<std::pair<const Outfit *, int>> ammo;
	
	int step;
	
	std::list<std::shared_ptr<Ship>> ships;
	std::list<Projectile> projectiles;
	std::list<Effect> effects;
	// Keep track of which ships we have not seen for long enough that it is
	// time to stop tracking their movements.
	std::map<std::list<Ship>::iterator, int> forget;
	
	std::list<ShipEvent> eventQueue;
	std::list<ShipEvent> events;
	// Keep track of who has asked for help in fighting whom.
	std::map<const Government *, std::weak_ptr<const Ship>> grudge;
	
	AsteroidField asteroids;
	double flash;
	bool doFlash;
	bool wasLeavingHyperspace;
	
	bool doClick = false;
	Point clickPoint;
	
	double load;
	int loadCount;
	double loadSum;
};



#endif
