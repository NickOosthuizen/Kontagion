#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::~StudentWorld()
{
	cleanUp();
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
}

void pickRandLoc(double& x, double& y)
{
	double angle = randInt(0, 359) * (PI / 180);
	double radius = randInt(0, 120);
	x = radius * cos(angle) + (VIEW_WIDTH / 2);
	y = radius * sin(angle) + (VIEW_HEIGHT / 2);
}

int StudentWorld::init()
{
	m_Soc = new Socrates(this);		//place socrates in level

	double startX;
	double startY;
	for (int i = 0; i < getLevel(); i++)
	{
		findStartLoc(startX, startY);
		m_gameObjects.push_back(new Pit(this, startX, startY));
	}
	for (int i = 0; i < min(5 * getLevel(), 25); i++)	//generate food objects first as they cannot overlap
	{
		findStartLoc(startX, startY);
		m_gameObjects.push_back(new Food(this, startX, startY));
		
	}
	for (int i = 0; i < max(180 - 20 * getLevel(), 20); i++)	//generate dirt piles according to level
	{
		findStartLoc(startX, startY);
		m_gameObjects.push_back(new DirtPile(this, startX, startY));
	}
	
    return GWSTATUS_CONTINUE_GAME;
}

void goodieStartLoc(double& x, double& y)	//generate Location of new goodie
{
	double  angle = randInt(0, 359) * (PI / 180);
	x = VIEW_RADIUS * cos(angle) + VIEW_WIDTH / 2;
	y = VIEW_RADIUS * sin(angle) + VIEW_HEIGHT / 2;
}

int StudentWorld::move()
{
	for (int i = 0; i < m_gameObjects.size(); i++)	
	{
		if (m_gameObjects[i]->isAlive())   //ask each object to dosomething if alive
			m_gameObjects[i]->doSomething();
		if (m_Soc->isAlive() == false)		//return if action causes socrates to die
		{
			delete m_Soc;
			m_Soc = nullptr;
			decLives();
			return GWSTATUS_PLAYER_DIED;
		}
	}
	for (int i = 0; i < m_gameObjects.size(); i++)
	{
		if (m_gameObjects[i]->isAlive() == false)		//remove any dead objects
		{
			delete m_gameObjects[i];
			m_gameObjects.erase(m_gameObjects.begin() + i);
			i--;
		}
	}

	bool levelCompleted = true;

	for (int i = 0; i < m_gameObjects.size(); i++)	//check if level still has pits or bacteria
	{
		if (m_gameObjects[i]->preventsLevelCompleting())
			levelCompleted = false;
	}

	if (levelCompleted)		//if none, finish level
	{
		playSound(SOUND_FINISHED_LEVEL);
		return GWSTATUS_FINISHED_LEVEL;
	}

	m_Soc->doSomething();		//ask socrates to do something

	double X, Y;	//goodie creation coordinates

	double chanceFungus = max(510 - getLevel() * 10, 200);	//determine whether or not to add a new fungus or goodie
	int makeFungus = randInt(0, chanceFungus);
	if (makeFungus == 0)
	{
		goodieStartLoc(X, Y);
		m_gameObjects.push_back(new Fungus(this, X, Y));
	}


	double chanceGoodie = max(510 - getLevel() * 10, 250);
	int makeGoodie = randInt(0, chanceGoodie);
	if (makeGoodie == 0)
	{
		goodieStartLoc(X, Y);
		int goodieType = randInt(0, 9);
		if (goodieType <= 5)
			m_gameObjects.push_back(new HealthGoodie(this, X, Y));
		else if (goodieType <= 8)
			m_gameObjects.push_back(new FlameThrowerGoodie(this, X, Y));
		else
			m_gameObjects.push_back(new ExtraLifeGoodie(this, X, Y));

	}

	ostringstream oss;		//update stats text line
	oss << "Score:  ";
	
	oss.fill('0');
	oss << setw(6) << getScore() << endl;
	oss << "  Level: " << getLevel();
	oss << "  Lives: " << getLives();
	oss << "  Health: " << m_Soc->health();
	oss << "  Sprays: " << m_Soc->sprayCharges();
	oss << "  Flames: " << m_Soc->flameCharges() << "       ";

	string stats = oss.str();
	setGameStatText(stats);


    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	for (int i = 0; i < m_gameObjects.size(); i++)	//delete all game objects
	{
		delete m_gameObjects[i];
	}
	if (m_Soc != nullptr)
		delete m_Soc;	//delete socrates
	m_gameObjects.clear();
}

bool StudentWorld::checkForOverLap(double x, double y, double distance)
{
	for (int i = 0; i < m_gameObjects.size(); i++)	//check gameobjects for if the chosen location overlaps with an existing object
	{
		if (m_gameObjects[i]->doesOverLap(x, y, distance) && m_gameObjects[i]->canOverLap() == false)
			return true;
	}
	return false;
}

void StudentWorld::findStartLoc(double& startX, double& startY)		//picks random start location until one that does not overlap is found (if overlap not allowed)
{
	do
	{
		pickRandLoc(startX, startY);
	} while (checkForOverLap(startX, startY, SPRITE_WIDTH));
}

bool StudentWorld::damageObject(double xLoc, double yLoc, double damage)	//damages objects with projectiles if projectile overlaps
{
	for (int i = 0; i < m_gameObjects.size(); i++)
	{
		if (m_gameObjects[i]->doesOverLap(xLoc, yLoc, SPRITE_WIDTH) && m_gameObjects[i]->isDamageable())	//check if overlaps with object that can be damaged
		{
			m_gameObjects[i]->takeDamage(damage);
			return true;
		}
	}
	return false;
}

void StudentWorld::addActor(Actor* a)
{
	m_gameObjects.push_back(a);
}

Actor* StudentWorld::getOverlappingEdible(Actor* a)	
{
	for (int i = 0; i < m_gameObjects.size(); i++)
	{
		if (m_gameObjects[i]->doesOverLap(a->getX(), a->getY(), SPRITE_WIDTH) && m_gameObjects[i]->isEdible())
			return m_gameObjects[i];
	}
	return nullptr;
}

bool StudentWorld::checkForMovePossible(double x, double y)
{
	double xDistance = VIEW_WIDTH / 2 - x;
	double yDistance = VIEW_HEIGHT / 2 - y;
	double distanceFromCenter = sqrt(xDistance * xDistance + yDistance * yDistance);
	if (distanceFromCenter > VIEW_RADIUS)	//if move would take object past view_radius from the center, it is not possible
		return false;
	for (int i = 0; i < m_gameObjects.size(); i++)		//cannot make move if it would tmake the object overlap with dirt
	{
		if (m_gameObjects[i]->doesOverLap(x, y, SPRITE_WIDTH / 2) && m_gameObjects[i]->blocksMovement())
			return false;
	}
	return true;
}

Socrates* StudentWorld::giveSocrates()
{
	return m_Soc;
}

bool StudentWorld::getAngleToNearestNearbyEdible(Actor* a, int dist, int& angle) const
{
	double shortestDistance = dist;
	bool edibleFound = false;
	for (int i = 0; i < m_gameObjects.size(); i++)
	{
		if (m_gameObjects[i]->isEdible())
		{
			double xDistance = (m_gameObjects[i]->getX() - a->getX());
			double yDistance = (m_gameObjects[i]->getY() - a->getY());
			double edibleDistance = sqrt(xDistance * xDistance + yDistance * yDistance);	//calculate distance from object to food
			if (edibleDistance <= shortestDistance)
			{
				shortestDistance = edibleDistance;	
				angle = atan2(yDistance, xDistance) * (180 / PI);	//set angle to angle between the object and the food
				edibleFound = true;
			}
		}
	}
	return edibleFound;
}

bool StudentWorld::getAngleToNearbySocrates(Actor* a, int dist, int& angle) const
{
	double xDistance = m_Soc->getX() - a->getX();
	double yDistance = m_Soc->getY() - a->getY();
	double socDistance = sqrt(xDistance * xDistance + yDistance * yDistance);	//calculate distance between object and socrates
	if (socDistance <= dist)	//if distance if within specified distance, set angle to angle between the object and socrates
	{
		angle = atan2(yDistance, xDistance) * (180 / PI);
		return true;
	}
	return false;
}