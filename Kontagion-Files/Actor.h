#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"
// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

 const double PI = 3.14159265359;

class Actor : public GraphObject
{
public:
	Actor(int imageID, double startX, double startY, int startDirection, int depth, StudentWorld* sw);
	bool isAlive() const;		//return current life status
	virtual void doSomething() = 0;		//a plain actor class will never be called to doSomething, this is the action classes take in each turn
	void setDead();		//modify status to dead
	bool doesOverLap(double x, double y, double distance) const;		//check if object's are close enough to be considered overlapping
	virtual bool canOverLap() const;	//default = false, return if object is allowed to overlap with other objects
	virtual bool isDamageable() const;	//default = true
	virtual bool blocksMovement() const;			//default = false, return if object cannot be moved onto
	virtual void takeDamage(int damage);		//default setsdead
	virtual bool isEdible() const;
	virtual bool preventsLevelCompleting() const;
	StudentWorld* giveWorld() const;	//accessor to object's world

		
		

private:
	bool m_alive;		//all objects need a world and start can be alive/dead
	StudentWorld* m_world;

};

class movingActor : public Actor
{
public:
	movingActor(int imageID, double startX, double startY, int startDirection, int depth, StudentWorld* sw, int startHealth);	//moving actors have health
	int health() const;	//return health
	virtual void takeDamage(int damage);	//reduce health	
	void restoreHealth();		//restore health to socrates's full
	virtual void playSoundHurt() const = 0;
	virtual void playSoundDie() const = 0;

private:
	int m_health;

};

class Socrates : public movingActor
{
public:
	Socrates(StudentWorld* sw);
	virtual void doSomething();
	void socratesMoveTo(double angle);	//socrates moves in a special circle around the edge
	void addFlame();
	virtual void playSoundHurt() const;
	virtual void playSoundDie() const;
	virtual bool isDamageable() const;
	int sprayCharges() const;
	int flameCharges() const;
	

private:
	int m_sprayCharges;
	int m_flameCharges;
};

class DirtPile : public Actor
{
public:
	DirtPile(StudentWorld* sw, double startX, double startY);
	virtual void doSomething();
	virtual bool canOverLap() const;	//dirt piles can overlap with each other
	virtual bool blocksMovement() const;	//movingactors cannot move over dirt

private:
};

class Food : public Actor
{
public:
	Food(StudentWorld* sw, double startX, double StartY);
	virtual void doSomething();
	virtual bool isDamageable() const;	//false
	virtual bool isEdible() const;	//can be eaten by bacteria
private:

};

class Pit : public Actor
{
public:
	Pit(StudentWorld* sw, double startX, double startY);
	virtual void doSomething();
	virtual bool isDamageable() const;	//pits can't be damaged
	virtual bool preventsLevelCompleting() const;

private:
	int m_salmonella;
	int m_aggressiveSalmonella;
	int m_eColi;
};

class Projectile : public Actor
{
public:
	Projectile(StudentWorld* sw, double startX, double StartY, double direction, double range, double damage, int type);
	virtual void doSomething();
	double giveDamage() const;
	virtual bool isDamageable() const;	//projectiles cannot be damaged by themselves

private:
	double m_range;		//projectiles have a max range, a damage, and a distance of how far they have traveled
	double m_damage;
	double m_distance;
};

class Spray : public Projectile
{
public:
	Spray(StudentWorld* sw, double startX, double startY, double direction);
private:

};

class Flame : public Projectile
{
public:
	Flame(StudentWorld* sw, double startX, double startY, double direction);
private:

};

class Goodie : public Actor
{
public:
	Goodie(StudentWorld* sw, double startX, double startY, int image);
	void doSomething();
	bool pickup(Socrates* s);
	virtual void goodieAction(Socrates* s) = 0;	//different goodie perform different specific actions
	bool lifeExceeded() const;	//check if lifeElapsed > lifeTime
	void liveLife();	//increment lifeTime

private:
	int m_lifeTime;
	int m_lifeElapsed;
};

class HealthGoodie : public Goodie
{
public:
	HealthGoodie(StudentWorld* sw, double startX, double startY);
	virtual void goodieAction(Socrates* s);
private:

};

class FlameThrowerGoodie : public Goodie
{
public:
	FlameThrowerGoodie(StudentWorld* sw, double startX, double startY);
	virtual void goodieAction(Socrates* s);
private:
};

class ExtraLifeGoodie : public Goodie
{
public: 
	ExtraLifeGoodie(StudentWorld* sw, double startX, double startY);
	virtual void goodieAction(Socrates* s);
private:
};

class Fungus : public Goodie
{
public: 
	Fungus(StudentWorld* sw, double startX, double startY);
	virtual void goodieAction(Socrates* s);
};

class Bacterium : public movingActor
{
public:
	Bacterium(StudentWorld* sw, int image, double startX, double startY, int health, int damage);
	virtual void doSomething();
	virtual bool preventsLevelCompleting() const;
	virtual void multiplyBacteria(double x, double y) = 0;	//multiplies each individual bacteria type
	virtual bool aggressiveSalmonellaSpecific();	//specific function for aggressive Salmonella do something
	virtual void bacteriumSpecific() = 0;		//function differs between bacteria types
	int movePlanDistance() const;	
	void decrementMovePlan();
	void resetMovePlan();	//change direction and reset move distance


private:
	int m_movePlanDistance;
	int m_foodEaten;
	int m_damage;
};

class Salmonella : public Bacterium
{
public:
	Salmonella(StudentWorld* sw, double startX, double startY, int health, int damage);
	virtual void bacteriumSpecific();
	virtual void playSoundHurt() const;
	virtual void playSoundDie() const;

private:

};

class RegularSalmonella : public Salmonella
{
public:
	RegularSalmonella(StudentWorld* sw, double startX, double startY);
	virtual void multiplyBacteria(double x, double y);

private:

};

class AggressiveSalmonella : public Salmonella
{
public:
	AggressiveSalmonella(StudentWorld* sw, double startX, double startY);
	virtual bool aggressiveSalmonellaSpecific();
	virtual void multiplyBacteria(double x, double y);

private:

};

class Ecoli : public Bacterium
{
public:
	Ecoli(StudentWorld* sw, double startX, double startY);
	virtual void bacteriumSpecific();
	virtual void playSoundHurt() const;
	virtual void playSoundDie() const;
	virtual void multiplyBacteria(double x, double y);

private:
};

#endif // ACTOR_H_
