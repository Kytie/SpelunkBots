#pragma once
#include<string>
using namespace std;

class ItemOfInterest
{
public:
	ItemOfInterest(string name, int posX, int posY);
	~ItemOfInterest();

	string getName();
	int getPosX();
	int getPosY();

private:
	string name;
	int posX;
	int posY;
};

