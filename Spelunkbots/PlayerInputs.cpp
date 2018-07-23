#include "stdafx.h"
#include "PlayerInputs.h"


PlayerInputs::PlayerInputs()
{
}


PlayerInputs::~PlayerInputs()
{
}

void PlayerInputs::CheckTerrain()
{
	descoveries.clear();
	for (int i = 0; i < 42; i++)
	{
		for (int j = 0; j < 34; j++)
		{
			CheckForItemOfInterest(map[i][j], i, j);
		}
	}
}

void PlayerInputs::CheckForItemOfInterest(int idNo, int itemPosX, int itemPosY)
{
	if (idNo == 3)
	{
		bool match = false;
		int x = 0;
		int y = 0;

		for (int i = 0; i < descoveries.size(); i++)
		{
			x = descoveries.at(i).getPosX()/16;
			y = descoveries.at(i).getPosY()/16;
			if (itemPosX == x && itemPosY == y)
			{
				match = true;
			}
		}

		if (match == false)
		{
			if (idNo == 3)
			{
				descoveries.push_back(ItemOfInterest("EXIT", itemPosX * 16, itemPosY * 16));
			}
		}
	}
}

void PlayerInputs::setPlayerPosX(int pPosX)
{
	playerPosX = pPosX;
}

void PlayerInputs::setPlayerPosY(int pPosY)
{
	playerPosY = pPosY;
}

void PlayerInputs::setPlayerFacing(int characterFacing)
{
	facing = characterFacing;
}

void PlayerInputs::SaveMap(double spmap[42][34])
{
	for (int i = 0; i < 42; i++)
	{
		for (int j = 0; j < 34; j++)
		{
			map[i][j] = spmap[i][j];
		}
	}
}

void PlayerInputs::DestroyDescoveries()
{
	descoveries.clear();
}

int PlayerInputs::getPlayerPosX()
{
	return playerPosX;
}

int PlayerInputs::getPlayerPosY()
{
	return playerPosY;
}

vector<ItemOfInterest> PlayerInputs::getDescoveries()
{
	return descoveries;
}

vector<double> PlayerInputs::getDescoveryLocation(string name)
{
	vector<double> location = { 0, 0 };
	for (int i = 0; i < descoveries.size(); i++)
	{
		if (name.compare(descoveries.at(i).getName()) == 0)
		{
			location.at(0) = descoveries.at(i).getPosX();
			location.at(1) = descoveries.at(i).getPosY();
		}
	}
	return location;
}

//this will find the first descovery in the level (that the player has seen) starting at the top left hand corner. Will not find nearest - needs upgrading.
double PlayerInputs::DistToDescovery()
{
	double distance = 0;
	double sqrX = 0;
	double sqrY = 0;
	vector<double> coords;
	//calculate distance between player and item of interest
	if (descoveries.size() != 0)
	{
		coords = getDescoveryLocation("EXIT");
		sqrX = playerPosX - coords.at(0);
		sqrY = playerPosY - coords.at(1);
	}
	else
	{
		sqrX = playerPosX - 672; //distance from right edge of map
		sqrY = playerPosY - 544; //distance from bottom of map
	}


	//square them
	sqrX = sqrX*sqrX;
	sqrY = sqrY*sqrY;;
	//find square root
	distance = sqrt(sqrX + sqrY);

	//normalise
	double mapSize = sqrt(mapHeightSqr + mapWidthSqr);
	distance = distance / mapSize;
	return distance;
}

double PlayerInputs::DirectionOfDescovery()
{
	double direction = 0;

	if (descoveries.size() != 0)
	{
		direction = playerPosX - descoveries.at(0).getPosX();
	}
	else
	{
		direction = playerPosX - 672;
	}

	if (direction > 0)
	{
		return -1.0; //-1 indicates descovery is to the left of agent.
	}
	if (direction < 0)
	{
		return 1.0; //1 indicates descovery is to the right of agent.
	}
	return 0;
}

double PlayerInputs::HeightOfDescovery()
{
	double height = 0;

	if (descoveries.size() != 0)
	{
		height = playerPosY - descoveries.at(0).getPosY();
	}
	else
	{
		height = playerPosY - 544;
	}

	if (height > 0)
	{
		return -1.0; //-1 indicates descovery is above the agent.
	}
	if (height < 0)
	{
		return 1.0; //1 indicates descovery is below the agent.
	}
	return 0;
}

double PlayerInputs::CheckForObstacle()
{
	bool threat = 0;
	bool safe = 1;
	int x = 0;
	int y = 0;

	if (facing != 0)
	{
		if (facing == 1)
		{
			x = floor((playerPosX + 16)/16); // this points to the block in front of the bot and gives around an 8 pixel gap to give the bot time to jump.
		}
		else if (facing == -1)
		{
			x = floor((playerPosX - 16) / 16); // this points to the block in front of the bot and gives around an 8 pixel gap to give the bot time to jump.
		}

		y = playerPosY / 16 + 1; //this points to the bots feet

		if (map[x][y] == 10 || map[x][y] == 0)
		{
			threat = 1;
			x = x + (facing * 2);

			if (x < 0)
			{
				x = 0;
			}
			if (x > 42)
			{
				x = 42;
			}
			if (map[x][y] == 1) // is the block 2 spaces ahead safe?
			{
				return safe = 1;
			}

			y = y - 1;
			if (y < 0)
			{
				y = 0;
			}
			if (y > 34)
			{
				y = 34;
			}
			if (map[x][y-1] == 1) // is the block 2 spaces ahead and 1 up safe?
			{
				return safe = 1;
			}

			y = y + 1;
			if (y < 0)
			{
				y = 0;
			}
			if (y > 34)
			{
				y = 34;
			}
			if (map[x][y+1] == 1) // is the block 2 spaces ahead and 1 down safe?
			{
				return safe = 1;
			}
		}

		//handle jumping over a barrier made by regular tarrain
		if (facing == 1)
		{
			x = floor((playerPosX + 16) / 16); // this points to the block in front of the bot and gives around an 8 pixel gap to give the bot time to jump.
		}
		else if (facing == -1)
		{
			x = floor((playerPosX - 16) / 16); // this points to the block in front of the bot and gives around an 8 pixel gap to give the bot time to jump.
		}
		if (x < 0)
		{
			x = 0;
		}
		if (x > 42)
		{
			x = 42;
		}

		y = playerPosY / 16; //this points to the bots feet
		if (y < 0)
		{
			y = 0;
		}
		if (y > 34)
		{
			y = 34;
		}

		if (map[x][y] == 1)
		{
			if (x + 1 < 0)
			{
				x = 0;
			}
			if (x + 1 > 42)
			{
				x = 42;
			}
			if (x + 2 < 0)
			{
				x = 0;
			}
			if (x + 2 > 42)
			{
				x = 42;
			}
			if (y + 1< 0)
			{
				y = 0;
			}
			if (y + 1 > 34)
			{
				y = 34;
			}
			if (y - 1 < 0)
			{
				y = 0;
			}
			if (y  - 1 > 34)
			{
				y = 34;
			}
			if (map[x + 1][y - 1] == 0 && map[x+2][y+1] != 0)
			{
				safe = true;
			}
			else
			{
				safe = false;
			}
		}
	}

	if (safe == false && threat == true)
	{
		return -1;
	}
	else if (safe == true && threat == true)
	{
		return 1;
	}
	return 0;
}
