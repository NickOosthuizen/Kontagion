#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <cmath>
#include <algorithm>


//Actor Functions

Actor::Actor(int imageID, double startX, double startY, int startDirection, int depth, StudentWorld* sw)
	: GraphObject(imageID, startX, startY, startDirection, depth)
{
	m_alive = true;		//start actors as alive in a studentworld
	m_world = sw;
}

bool Actor::isAlive() const
{
	return m_alive;
}

void Actor::setDead()
{
	m_alive = false;
}

bool Actor::doesOverLap(double x, double y, double distance) const
{
	double dist = sqrt((getX() - x) * (getX() - x) + (getY() - y) * (getY() - y));	//get distance between object and specified x and y
	if (dist <= distance)
		return true;
	else
		return false;
}

bool Actor::isDamageable() const
{
	return true;
}

StudentWorld* Actor::giveWorld() const
{
	return m_world;
}

bool Actor::canOverLap() const
{
	return false;
}

bool Actor::blocksMovement() const
{
	return false;
}

void Actor::takeDamage(int damage)
{
	setDead();
}

bool Actor::isEdible() const
{
	return false;
}

bool Actor::preventsLevelCompleting() const
{
	return false;
}


//Moving Actor Functions

movingActor::movingActor(int imageID, double startX, double startY, int startDirection, int depth, StudentWorld* sw, int startHealth)
	: Actor(imageID, startX, startY, startDirection, depth, sw)
{
	m_health = startHealth;
}

int movingActor::health() const
{
	return m_health;
}

void movingActor::takeDamage(int damage)
{
	m_health -= damage;
	if (m_health > 0)
		playSoundHurt();
	return;
}

void movingActor::restoreHealth()	//restore socrates to full health
{
	m_health = 100;
}


//Socrates Functions

Socrates::Socrates(StudentWorld* sw)
	:movingActor(IID_PLAYER, 0, VIEW_HEIGHT/2, 0, 0, sw, 100)
{
	m_sprayCharges = 20;
	m_flameCharges = 5;
}

 void Socrates::socratesMoveTo(double angle)
{
	 double oldTheta = atan2((getY()-VIEW_RADIUS), (getX() - VIEW_RADIUS));		//calculate Socrate's positional angle
	 double theta = oldTheta + (angle * (PI / 180));			//calculate positional angle after move
	 moveTo(VIEW_RADIUS * cos(theta) + VIEW_RADIUS, VIEW_RADIUS * sin(theta) + VIEW_RADIUS);	//move Socrates to new angle
	 setDirection((theta * 180 / PI) + 180);	//adjust direction to face center of circle
}

 void Socrates::addFlame()
 {
	 m_flameCharges += 5;
 }

 void Socrates::playSoundHurt() const
 {
	 giveWorld()->playSound(SOUND_PLAYER_HURT);
 }

 void Socrates::playSoundDie() const
 {
	 giveWorld()->playSound(SOUND_PLAYER_DIE);
 }

void Socrates::doSomething()
{
	if (health() <= 0)	//if socrates dies, return
	{
		playSoundDie();
		setDead();
		return;
	}
	int key;
	if (giveWorld()->getKey(key))		//get key press and perform action depending on press
	{
		switch (key)
		{
		case KEY_PRESS_LEFT:	//rotate socrates counterclockwise
			socratesMoveTo(5);
			break;
		case KEY_PRESS_RIGHT:	//rotate socrates clockwise
			socratesMoveTo(-5);
			break;
		case KEY_PRESS_SPACE:	//fire spray
			if (m_sprayCharges > 0)
			{
				giveWorld()->playSound(SOUND_PLAYER_SPRAY);
				double newX, newY;
				getPositionInThisDirection(getDirection(), SPRITE_WIDTH, newX, newY);		//spawn the spray one sprite_width in front of socrates
				giveWorld()->addActor(new Spray(giveWorld(), newX, newY, getDirection()));
				m_sprayCharges--;
			}
			break;
		case KEY_PRESS_ENTER:	//fire flames
			if (m_flameCharges > 0)
			{
				giveWorld()->playSound(SOUND_PLAYER_FIRE);
				for (int i = 0; i < 16; i++)		//add 16 flames in a circle around socrates
				{
					int dir = getDirection();
					if (dir += (i * 22) > 359)
						dir = dir + (i * 22) - 360;
					else
						dir += (i * 22);
					double newX, newY;
					getPositionInThisDirection(dir, SPRITE_WIDTH, newX, newY);
					giveWorld()->addActor(new Flame(giveWorld(), newX, newY, dir));
				}
				m_flameCharges--;
			}
			break;
		default:
		
			break;
		}
		return;
	}
	if (m_sprayCharges < 20)	//if no key press detected, increase spraycharges
		m_sprayCharges++;

	return;
}

bool Socrates::isDamageable() const
{
	return false;
}

int Socrates::sprayCharges() const
{
	return m_sprayCharges;
}

int Socrates::flameCharges() const
{
	return m_flameCharges;
}


//Dirtpile functions

DirtPile::DirtPile(StudentWorld* sw, double startX, double startY)
	:Actor(IID_DIRT, startX, startY, 0, 1, sw)
{}

void DirtPile::doSomething()
{
	return;
}

bool DirtPile::canOverLap() const	//dirt piles can overlap with each other
{
	return true;
}

bool DirtPile::blocksMovement() const
{
	return true;
}



//Food functions

Food::Food(StudentWorld* sw, double startX, double startY)
	:Actor(IID_FOOD, startX, startY, 90, 1, sw)
{}

void Food::doSomething()
{}

bool Food::isDamageable() const
{
	return false;
}

bool Food::isEdible() const
{
	return true;
}



//Pit functions

Pit::Pit(StudentWorld* sw, double startX, double startY)
	:Actor(IID_PIT, startX, startY, 0, 1, sw)
{
	m_salmonella = 5;
	m_aggressiveSalmonella = 3;
	m_eColi = 2;
}

void Pit::doSomething()
{
	if (m_salmonella <= 0 && m_aggressiveSalmonella <= 0 && m_eColi <= 0)
	{
		setDead();
		return;
	}
	int chanceBacteria = randInt(0, 49);	//1/50 chance of a new bacterium
	if (chanceBacteria == 0)
	{
		while (true)
		{
			int typeBacteria = randInt(0, 2);	//randomly pick an integer 0 - 2, and then try to add a bacterium of the integer
			if (typeBacteria == 0 && m_salmonella > 0)	//if there are no more of that type, pick another integer
			{
				giveWorld()->addActor(new RegularSalmonella(giveWorld(), getX(), getY()));	
				m_salmonella--;
				return;
			}
			else if (typeBacteria == 1 && m_aggressiveSalmonella > 0)
			{
				giveWorld()->addActor(new AggressiveSalmonella(giveWorld(), getX(), getY()));
				m_aggressiveSalmonella--;
				return;
			}
			else if (typeBacteria == 2 && m_eColi > 0)
			{
				giveWorld()->addActor(new Ecoli(giveWorld(), getX(), getY()));
				m_eColi--;
				return;
			}

		}

	}
}

bool Pit::isDamageable() const
{
	return false;
}

bool Pit::preventsLevelCompleting() const
{
	return true;
}




//Projectile functions

Projectile::Projectile(StudentWorld* sw, double startX, double startY, double direction, double range, double damage, int image)
	: Actor(image, startX, startY, direction, 1, sw)
{
	m_range = range;	
	m_damage = damage;
	m_distance = 0;
}

void Projectile::doSomething()
{
	if (!isAlive())	
		return;
	if (giveWorld()->damageObject(getX(), getY(), giveDamage()))	//set projectile dead if it deals damage
		setDead();
	moveAngle(getDirection(), SPRITE_WIDTH);	//move in straight direction
	m_distance += (SPRITE_WIDTH);
	if (m_distance >= m_range)	//set dead if range is exceeded
	{
		setDead();
		return;
	}
}

double Projectile::giveDamage() const
{
	return m_damage;
}

bool Projectile::isDamageable() const
{
	return false;
}



Spray::Spray(StudentWorld* sw, double startX, double startY, double direction)
	:Projectile(sw, startX, startY, direction, 112, 2, IID_SPRAY)
{}



Flame::Flame(StudentWorld* sw, double startX, double startY, double direction)
	:Projectile(sw, startX, startY, direction, 32, 5, IID_FLAME)
{}



//Goodie Functions

Goodie::Goodie(StudentWorld* sw, double startX, double startY, int image)
	:Actor(image, startX, startY, 0, 1, sw)
{
	m_lifeElapsed = 0;				
	m_lifeTime = std::max(randInt(0, 300 - 10 * sw->getLevel() - 1), 50);
}

void Goodie::doSomething()
{
	if (!isAlive())
		return;
	if (pickup(giveWorld()->giveSocrates()))	//if socrates has picked up the goodie, return
		return;
	if (lifeExceeded())		//check if goodie needs to dissapear
	{
		setDead();
		return;
	}
	liveLife();
	return;
}

bool Goodie::pickup(Socrates* s)
{
	if (doesOverLap(s->getX(), s->getY(), SPRITE_WIDTH))	//check if socrates has picked up the goodie
	{
		goodieAction(s);		//perform action specific to goodie type
		setDead();
		return true;
	}
	return false;
}

bool Goodie::lifeExceeded() const		//return true if the object has existed longer than its lifetime
{
	if (m_lifeElapsed > m_lifeTime)
		return true;
	return false;
}

void Goodie::liveLife()	//increments lifeelapsed by 1 per tick
{
	m_lifeElapsed++;
}



//Health Goodie Functions

HealthGoodie::HealthGoodie(StudentWorld* sw, double startX, double startY)
	:Goodie(sw, startX, startY, IID_RESTORE_HEALTH_GOODIE)
{}

void HealthGoodie::goodieAction(Socrates* s)
{
	giveWorld()->increaseScore(250);
	s->restoreHealth();
	giveWorld()->playSound(SOUND_GOT_GOODIE);
}



//FlameThrower Goodie functions

FlameThrowerGoodie::FlameThrowerGoodie(StudentWorld* sw, double startX, double startY)
	:Goodie(sw, startX, startY, IID_FLAME_THROWER_GOODIE)
{}

void FlameThrowerGoodie::goodieAction(Socrates* s)
{
	giveWorld()->increaseScore(300);
	s->addFlame();
	giveWorld()->playSound(SOUND_GOT_GOODIE);
}




//Extralife goodie functions

ExtraLifeGoodie::ExtraLifeGoodie(StudentWorld* sw, double startX, double startY)
	:Goodie(sw, startX, startY, IID_EXTRA_LIFE_GOODIE)
{}

void ExtraLifeGoodie::goodieAction(Socrates* s)
{
	giveWorld()->increaseScore(500);
	giveWorld()->incLives();
	giveWorld()->playSound(SOUND_GOT_GOODIE);
}




//Fungus functions

Fungus::Fungus(StudentWorld* sw, double startX, double startY)
	:Goodie(sw, startX, startY, IID_FUNGUS)
{}

void Fungus::goodieAction(Socrates* s)
{
	giveWorld()->increaseScore(-50);
	s->takeDamage(20);
}




//Bacterium functions

Bacterium::Bacterium(StudentWorld* sw, int image, double startX, double startY, int health, int damage)
	:movingActor(image, startX, startY, 90, 0, sw, health)
{
	m_movePlanDistance = 0;
	m_foodEaten = 0;
	m_damage = damage;
	sw->playSound(SOUND_BACTERIUM_BORN);
}

void Bacterium::doSomething()
{
	if (health() <= 0)	//if bacteria runs out of health, it dies, increases score, plays it death sound, and checks if it should leave food
	{
		playSoundDie();
		giveWorld()->increaseScore(100);
		setDead();
		int chanceFood = randInt(0, 1);
		if (chanceFood == 0)
			giveWorld()->addActor(new Food(giveWorld(), getX(), getY()));
		return;
	}
	bool performedAggressive = aggressiveSalmonellaSpecific();	//aggressive salmonella has a specific action that determines its later behavior
	if (giveWorld()->giveSocrates()->doesOverLap(getX(), getY(), SPRITE_WIDTH))	//damage socrates if bacteria overlaps with him
	{
		giveWorld()->giveSocrates()->takeDamage(m_damage);
	}
	else
	{
		if (m_foodEaten == 3)	//multiply if bacteria has eaten 3 food
		{
			double newX = getX();
			double newY = getY();

			if (newX < VIEW_WIDTH / 2)
				newX += SPRITE_WIDTH / 2;
			else if (newX > VIEW_WIDTH / 2)
				newX -= SPRITE_WIDTH / 2;
			if (newY < VIEW_HEIGHT / 2)
				newY += SPRITE_WIDTH / 2;
			else if (newY > VIEW_HEIGHT / 2)
				newY -= SPRITE_WIDTH / 2;

			multiplyBacteria(newX, newY);
			m_foodEaten = 0;
		}
		else                   //otherwise check if bacteria currently overlaps with food, and if so eat food
		{
			Actor* food = giveWorld()->getOverlappingEdible(this);
			if (food != nullptr)
			{
				food->setDead();
				m_foodEaten++;
			}
		}
	}
	if (!performedAggressive)	//perform bacterium specific action, if an aggressive salmonella has performed its specific action, it will not
		bacteriumSpecific();
	return;

}

bool Bacterium::preventsLevelCompleting() const
{
	return true;
}

bool Bacterium::aggressiveSalmonellaSpecific()	//only aggressiveSalmonella perform an action in this position
{
	return false;
}

int Bacterium::movePlanDistance() const
{
	return m_movePlanDistance;
}

void Bacterium::decrementMovePlan()
{
	if (m_movePlanDistance > 0)
		m_movePlanDistance--;
}

void Bacterium::resetMovePlan()		//find a new location to move
{
	int newAngle = randInt(0, 359);
	setDirection(newAngle);
	m_movePlanDistance = 10;
}

//salmonella functions

Salmonella::Salmonella(StudentWorld* sw, double startX, double startY, int health, int damage)
	:Bacterium(sw, IID_SALMONELLA, startX, startY, health, damage)
{}

void Salmonella::bacteriumSpecific()
{
	if (movePlanDistance() > 0)		//if bacteria has  a plan to move in a direction, attempt move
	{
		decrementMovePlan();
		double newX;
		double newY;
		getPositionInThisDirection(getDirection(), 3, newX, newY);
		if (giveWorld()->checkForMovePossible(newX, newY))	//move if possible, pick new direction if not
		{
			moveAngle(getDirection(), 3);
			return;
		}
		else
		{
			resetMovePlan();
			return;
		}
	}
	else
	{
		int edibleAngle;	//if it does not have a plan, find nearby food and try to move toward it
		if (giveWorld()->getAngleToNearestNearbyEdible(this, VIEW_RADIUS, edibleAngle) == true)
		{
			setDirection(edibleAngle);
			double newX, newY;
			getPositionInThisDirection(getDirection(), 3, newX, newY);
			if (giveWorld()->checkForMovePossible(newX, newY) == true)
			{
				moveAngle(getDirection(), 3);
				return;
			}
			else
			{
				resetMovePlan();
				return;
			}
		}
		else    //if no food, pick random direction to move
		{
			resetMovePlan();
			return;
		}
	}

}



void Salmonella::playSoundHurt() const
{
	giveWorld()->playSound(SOUND_SALMONELLA_HURT);
}

void Salmonella::playSoundDie() const
{
	giveWorld()->playSound(SOUND_SALMONELLA_DIE);
}



//regular salmonella functions

RegularSalmonella::RegularSalmonella(StudentWorld* sw, double startX, double startY)
	:Salmonella(sw, startX, startY, 4, 1)
{}

void RegularSalmonella::multiplyBacteria(double x, double y)	
{
	giveWorld()->addActor(new RegularSalmonella(giveWorld(), x, y));
}


//aggressive salmonella functions

AggressiveSalmonella::AggressiveSalmonella(StudentWorld* sw, double startX, double startY)
	:Salmonella(sw, startX, startY, 10, 2)
{}

bool AggressiveSalmonella::aggressiveSalmonellaSpecific()
{
	int theta;							//find socrates and try to move towards him if he is range
	if (giveWorld()->getAngleToNearbySocrates(this, 72, theta))
	{
		double newX, newY;
		getPositionInThisDirection(theta, 3, newX, newY);
		if (giveWorld()->checkForMovePossible(newX, newY))
			moveAngle(theta, 3);
		return true;
	}
	return false;
}

void AggressiveSalmonella::multiplyBacteria(double x, double y)
{
	giveWorld()->addActor(new AggressiveSalmonella(giveWorld(), x, y));
}

//ecoli functions

Ecoli::Ecoli(StudentWorld* sw, double startX, double startY)
	:Bacterium(sw, IID_ECOLI, startX, startY, 5, 4)
{}

void Ecoli::bacteriumSpecific()
{
	int theta;
	if (giveWorld()->getAngleToNearbySocrates(this, VIEW_WIDTH, theta) == true)		//find socrates and try to move toward him
	{
		setDirection(theta);
		for (int i = 0; i < 10; i++)		//if stuck after 10 adjustments, stop trying to move
		{
			double newX, newY;
			getPositionInThisDirection(getDirection(), 2, newX, newY);
			if (giveWorld()->checkForMovePossible(newX, newY) == true)
			{
				moveAngle(getDirection(), 2);
				return;
			}
			if ((theta += 10) >= 359)			//wrap around theta if angle exceeds 359
			{
				theta = (theta + 10 - 360);
			}
			else
				theta += 10;
			setDirection(theta);
		}
		return;
	}

}

void Ecoli::playSoundHurt() const
{
	giveWorld()->playSound(SOUND_ECOLI_HURT);
}

void Ecoli::playSoundDie() const
{
	giveWorld()->playSound(SOUND_ECOLI_DIE);
}

void Ecoli::multiplyBacteria(double x, double y)
{
	giveWorld()->addActor(new Ecoli(giveWorld(), x, y));
}