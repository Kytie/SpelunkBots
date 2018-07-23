#pragma once
#include<iostream>
#include<string>
#include<vector>
#include"ItemOfInterest.h"

using namespace std;

class PlayerInputs
{
public:
	PlayerInputs();
	~PlayerInputs();
	void CheckTerrain();
	void setPlayerPosX(int playerPosX);
	void setPlayerPosY(int playerPosY);
	void setPlayerFacing(int facing);
	void SaveMap(double map[42][34]);
	void DestroyDescoveries();
	int getPlayerPosX();
	int getPlayerPosY();
	double DistToDescovery();
	double DirectionOfDescovery();
	double HeightOfDescovery();
	double CheckForObstacle();
	vector<double> getDescoveryLocation(string name);
	vector<ItemOfInterest> getDescoveries();

private:
	const int mapWidthSqr = 451584; //Total map width squared
	const int mapHeightSqr = 295936; //Total map height squared
	int playerPosX = 0;
	int playerPosY = 0;
	int previousY = -1;
	int facing = 0;
	vector<ItemOfInterest> descoveries;
	double map[42][34];

	void CheckForItemOfInterest(int idNo, int itemPosX, int itemPosY);
	
};

