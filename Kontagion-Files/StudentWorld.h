#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <vector>

class Actor;
class Projectile;
class Socrates;
class Bacterium;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
class Actor;
class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
	~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
	bool checkForOverLap(double x, double y, double distance);	//check if objects overlap at given location
	void findStartLoc(double& startX, double& startY);		//find an init location that does not overlap where not allowed
	bool damageObject(double xLoc, double yLoc, double damage);		//damage an object with projectile
	void addActor(Actor* a);
	Actor* getOverlappingEdible(Actor* a);		//return overlapping food
	bool checkForMovePossible(double x, double y);	//check if a move isblocked by dirt
	Socrates* giveSocrates();
	bool getAngleToNearestNearbyEdible(Actor* a, int dist, int& angle) const;	//find angle to nearest food within distance
	bool getAngleToNearbySocrates(Actor* a, int dist, int& angle) const;	//find angle to socrates within distance

private:
	Socrates* m_Soc = nullptr;
	std::vector<Actor*> m_gameObjects;
};

void pickRandLoc(double& x, double& y);
void goodieStartLoc(double& X, double& Y);


#endif // STUDENTWORLD_H_
