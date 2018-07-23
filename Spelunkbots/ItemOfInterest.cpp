#include "stdafx.h"
#include "ItemOfInterest.h"


ItemOfInterest::ItemOfInterest(string name, int x, int y)
{
	this->name = name;
	posX = x;
	posY = y;
}


ItemOfInterest::~ItemOfInterest()
{
}

string ItemOfInterest::getName()
{
	return name;
}

int ItemOfInterest::getPosX()
{
	return posX;
}

int ItemOfInterest::getPosY()
{
	return posY;
}

