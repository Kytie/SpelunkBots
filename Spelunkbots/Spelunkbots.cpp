// Spelunkbots.cpp : Defines the exported functions for the DLL application.
// Spelunkbots Source code written by Daniel Scales t:@DanielCake
// ANN and Genetic Algorithm code writtain by James Kyte james_kyte_36@hotmail.com

#include "stdafx.h"
#include "Enemy.h"
#include "Altar.h"
#include "PlayerInputs.h"
#include "Chromosome.h"
#include "ANN.h"
#include "Fitness.h"
#include "BreedingTools.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <list>
#include <map>
#include <thread>

// entry point
BOOL WINAPI DllMain(
  HANDLE hinstDLL, 
  DWORD dwReason, 
  LPVOID lpvReserved
)
{
	AllocConsole();
	freopen("CON", "w", stdout);

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

struct collectableObject {
	int type;
	int id;
	double x;
	double y;
};

struct enemyObject {
	int type;
	int id;
	double x;
	double y;
	double directionFacing;
	double status;
};

int screenTop;
int screenBottom;
int screenWidth;
int screenHeight;

// hold arrays of each type of object in here 
// means user can query the nearest object, or all of them
// and if it is 0, c++ can safely return false
// which is a lot safer than GML (GameMaker language).

// different states held in this, reset each time a new level is loaded

// once this is working, adapt to mark if it has a ladder in - essential
// for navigation

// map holds a bool as to whether there is a land tile in that location
// fog holds a bool as to whether the node has been discovered
// X = 41, Y = 33
double spmap[42][34];
double mapLiquids[42][34];
double mapFog[42][34];

// An array that contains how many spider webs each position contains
double spiderWebs[42][34];
double pushBlocks[42][34];
double bats[42][34];

std::vector<collectableObject> collectablesList;
std::vector<collectableObject> enemiesList;


double hasResetMap = 0;

double screenX;
double screenY;
double screenW;
double screenH;
////////////////////////////////////////////////////////////////////////////

//ANN
int inputs = 4;
int hiddenLayers = 2;
int hidden = 4;
int outputs = 4;
ANN neuralNetwork(inputs, hiddenLayers, hidden, outputs); //do not edit

//number of chromosomes
int geneNum = (inputs*hidden) + ((hidden*hidden)*(hiddenLayers - 1)) + (outputs*hidden); //do not edit

//playerInputs
PlayerInputs pInput; //do not edit

//moves
vector<double> moves(4); //do not edit
vector<double> outputData(3); //do not edit

//number of tests per population member
int maxTests = 2;
int testsDone = 0; //do not edit

//population
int maxPopulation = 1;
vector<Chromosome> population; //do not edit
int popMemTested = 0; //do not edit
int mutationProb = 30; //30% chance of mutation
int crossoverProb = 30; //30% of crossover breeding
bool seededStart = true; //check for whether you want a seeded start

//number of generations
int generations = 1;
int generationsTested = 0; //do not edit

//number of parents
int parentNum = 1; //must pick a number that can be devided into the maxPopulation without a remainder.

//levels
vector<int> levelTimes = {4, 4}; //contains the maximum time allocated for level completion. Adjust times for level being tested.
vector<char*> exitLevels = { "FIND DOOR", "FIND DOOR 2", "FIND DOOR 3", "FIND DOOR 4", "FIND DOOR 5", "FIND DOOR 6", "FIND DOOR 7", "FIND DOOR 8", "FIND DOOR 9", "FIND DOOR 10" }; //vector for containing all the exit level names - this is used to check for which fitness test to run. 
vector<char*> levels = { "FIND DOOR", "FIND DOOR 2", "" }; //vector for containg the levels to be run - always have a "" at the end of the vector to end testing otherwise application will crash.
double seconds = 0.0; // seconds left to complete level. //do not edit

//fitness
Fitness fitnessFunction; //do not edit
char * testName = ""; //hold the name of the fitness function test. do not edit
bool parentTest = true; //are you testing the parents?

//breeding tools
BreedingTools breeding(maxPopulation, parentNum, mutationProb, crossoverProb, geneNum); //do not edit

//sync booleans
bool ready = false; //do not edit
bool nextLevel = false; //do not edit
int levelNum = 0; //do not edit

//time
time_t startTime = NULL; //do not edit

//file names
int experimentNumber = 1; //denotes the experiment number we are currently on with regards to the filenames stored in GenerationStats. 1 experiment consists of running all the test levels. //do not edit

/////////////////////////////////////////////////////////////////////////////
// Player equipment
bool udjatEye;
bool coolGlasses;

// Game State
bool shopkeepersAngered;

#define GMEXPORT extern "C" __declspec (dllexport)

using namespace std;

GMEXPORT double SetScreenXYWH(double x, double y, double w, double h)
{
	screenX = x;
	screenY = y;
	screenW = w;
	screenH = h;
	return 0;
}

GMEXPORT double SampleFunction(double a, double b) {
return a * b;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CheckFitnessTest()
{
	for (int i = 0; i < exitLevels.size(); i++)
	{
		if (levels.at(levelNum) == exitLevels.at(i))
		{
			if (fitnessFunction.getFitnessTest() != "EXIT")
			{
				return "EXIT";
			}
			else
			{
				return "";
			}
		}
	}
	return "NULL"; //return null if no level can be matched to a fitness function.
}

void SaveParents()
{
	ofstream fileStream;
	ostringstream concat;
	string directoryName = "";

	concat.str("");
	concat << "..\\..\\GenerationStats\\Experiment-" << experimentNumber << "\\Saved-Parents-Level-" << levelNum << ".txt";
	directoryName = concat.str();
	fileStream.open(directoryName.c_str(), ofstream::out);
	if (fileStream.is_open())
	{
		for (int i = 0; i < parentNum; i++)
		{
			fileStream << "START" << endl;
			for (int j = 0; j < population.at(i).getSize(); j++)
			{
				fileStream << population.at(i).getGene(j) << endl;
			}
			fileStream << "END" << endl;
		}
	}
	else
	{
		cout << "Unable to write parents data to file." << endl;
	}
	fileStream.close();
}

bool SeededStart()
{
	bool done = 0;
	bool failed = 0;
	string input = "";
	vector<string> genes;
	ifstream fileStream;
	string directoryName = "..\\..\\SeededStart\\Saved-Parents.txt";

	fileStream.open(directoryName);
	if (fileStream.is_open())
	{
		while (fileStream.good())
		{
			getline(fileStream, input);
			if (strcmp(input.c_str(), "START") == 0)
			{
				done = 0;
				while (!done)
				{
					getline(fileStream, input);
					if (strcmp(input.c_str(), "END") != 0)
					{
						genes.push_back(input);
						
					}
					else
					{
						done = 1;
					}
				}
				if (genes.size() == geneNum)
				{
					Chromosome chromosome(geneNum);
					for (int i = 0; i < genes.size(); i++)
					{
						chromosome.setGene(i, stod(genes.at(i)));
					}
					population.push_back(chromosome);
					genes.clear();
				}
				else
				{
					cout << "Cannot import Chromosomes as they have a number of genes that is different to the required number." << endl;
					population.clear(); //get rid of any parents that were imported as some are missing, this will create a fresh group.
					genes.clear();
					failed = true;
				}
			}
		}
		if (!failed)
		{
			if (!parentTest)
			{
				if (population.size() >= parentNum)
				{
					breeding.BeginBreeding(population, fitnessFunction.getAverageFitness(), 1);
					population.clear();
					population = breeding.getNewPopulation();
				}
				else
				{
					cout << "Cannot create seeded population as seed file does not contain enough parents. If you want to use these parents please lower the parent number variable." << endl;
					population.clear(); //get rid of any parents that were imported as some are missing, this will create a fresh group.
					failed = true;
				}
			}
		}
	}
	else
	{
		cout << "Unable to open Saved-Parents.txt file in SeededStart Folder." << endl;
		failed = 1;
	}
	fileStream.close();
	return failed;
}

GMEXPORT double SaveMap()
{
	pInput.SaveMap(spmap);
	return 0;
}

GMEXPORT double CheckTerrain()
{
	pInput.CheckTerrain();
	return 0;
}

GMEXPORT double StartingPosition(double x, double y)
{
	fitnessFunction.setStartPosition(x, y);
	return 0;
}

GMEXPORT double Debug(char* text, double val)
{
	cout << text << ": " << val << endl;
	return 0;
}

GMEXPORT double DebugS(char* text, char* text2)
{
	cout << text << text2 << endl;
	return 0;
}

GMEXPORT double Initialise() //creates population, sets up the ANN and sets up the fitness function to use.
{
	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED); //sets flags for execution state and should stop a bug where windows would go idle and suck CPU power away from process.
	cout.precision(16);
	bool failed = 0;
	if (seededStart)
	{
		failed = SeededStart();
		for (int i = 0; i < population.size(); i++)
		{
			for (int j = 0; j < population.at(i).getSize(); j++)
			{
				population.at(i).getGene(j);
			}
		}
	}
	if (!seededStart || failed)
	{
		for (int i = 0; i < maxPopulation; i++)
		{
			Chromosome chromosome(geneNum);
			chromosome.CreateGenes();
			population.push_back(chromosome);
		}
	}
	neuralNetwork.setWeights(population.at(0));
	fitnessFunction.UpdateFitnessTest(CheckFitnessTest());
	cout << "Population, ANN and Fitness Testing Initialised." << endl;
	cout << "Population Members: " << population.size() << endl << endl;

	//Create file structure for storing tests. CreateDirectoryA simply fails if the folder already exists so essentially these are checking if folders are there and if not are creating them. 
	string directoryName = "..\\..\\GenerationStats";
	CreateDirectoryA(directoryName.c_str(), NULL);
	if (ERROR_PATH_NOT_FOUND == GetLastError()) //if ERROR_PATH_NOT_FOUND is != GetLastError this measn that GetLastError is probably equal to either ERROR_ALREADY_EXISTS or nothing. If it is either of these two we can create inner folders 
	{
		cout << "Unable to Create Generation Statistics Directory, Please Check Filepath - from spelunky_1_1 folder folder it should be ..\\..\\GenerationStats." << endl;
	}

	directoryName = "..\\..\\SeededStart";
	CreateDirectoryA(directoryName.c_str(), NULL);
	if (ERROR_PATH_NOT_FOUND == GetLastError()) //if ERROR_PATH_NOT_FOUND is != GetLastError this measn that GetLastError is probably equal to either ERROR_ALREADY_EXISTS or nothing. If it is either of these two we can create inner folders 
	{
		cout << "Unable to Create Seeded Start Directory, Please Check Filepath - from spelunky_1_1 folder folder it should be ..\\..\\SeededStart." << endl;
	}
	return 0;
}

GMEXPORT double CalculateMoves(double x, double y)
{
	//modifying x and y as they are not using the same point of origin mesurements as the the other models are.
	x = x - 8;
	y = y - 8;

	ready = false;
	pInput.setPlayerPosX(x);
	pInput.setPlayerPosY(y);
	neuralNetwork.UpdateInputs(pInput);
	neuralNetwork.Update();

	moves.at(0) = 0;//go right
	moves.at(1) = 0;//go left
	moves.at(2) = 0;//jump
	moves.at(3) = 0;//look up
	outputData = neuralNetwork.getOutputs();
	//if you are roughly at the same place as the objective, look up.
	if (outputData.at(0) < 0.1 && outputData.at(0) > -0.1)
	{
		//look up
		moves.at(3) = 1;
	}

	if (outputData.at(1) > 0) // go right
	{
		if (outputData.at(3) >=0)
		{
			//go right
			moves.at(0) = 1;
			pInput.setPlayerFacing(1);
		}
		else if (outputData.at(3) < 0)
		{
			//go left
			moves.at(1) = 1;
			pInput.setPlayerFacing(-1);
		}
	}

	if (outputData.at(1) < 0) // go left
	{
		if (outputData.at(3) >= 0)
		{
			//go left
			moves.at(1) = 1;
			pInput.setPlayerFacing(-1);
		}
		else if (outputData.at(3) < 0)
		{
			//go right
			moves.at(0) = 1;
			pInput.setPlayerFacing(1);
		}
	}

	if (outputData.at(3) > 0.8)
	{
		//jump
		moves.at(2) = 1;
	}

	if (outputData.at(2) < 0)
	{
		//jump
		moves.at(2) = 1;
	}

	ready = true;
	return 0;
}

GMEXPORT double GetMoves(double index)
{
	//does nothing and makes game maker wait until moves are ready to be given by the ANN.
	while (ready == false)
	{
	}
	return moves.at(index);
}

GMEXPORT double CalculateFitness(double x, double y)
{
	testsDone++; //increate number of tests done.
	startTime = NULL; //reset start time as level/test has ended.

	//modifying x and y as they are not using the same position mesurements as the items are.
	x = x - 8;
	y = y - 8;
	fitnessFunction.CalculateFitness(seconds, levelTimes.at(levelNum), x, y, &pInput); //using seconds because that should tell us how quickly the bot completed the level.
	if (testsDone == maxTests)
	{
		fitnessFunction.CalculateAverageFitness();
	}
	return 0;
}

GMEXPORT double CheckANN()
{
	if (testsDone == maxTests)
	{
		popMemTested++;
		cout << "Complete: " << ((((double)generationsTested*((double)maxTests*(double)maxPopulation)) + ((double)popMemTested*(double)maxTests)) / (generations*maxPopulation*maxTests)) * 100 << "%" << endl;
		cout << "Population Members Tested: " << popMemTested << endl << endl;
		if (popMemTested == maxPopulation)
		{
			cout << "Max Population Tested - now breeding." << endl << endl;
			generationsTested++;
			cout << "Generations Tested: " << generationsTested << endl;
			experimentNumber = fitnessFunction.getStats(experimentNumber, levelNum, parentTest); // we get the experiment number back from this function has it helps us keep track of how many experiments have been run via the storage folder numbers.
			if (generationsTested == generations)
			{
				nextLevel = true;
				generationsTested = 0;
			}
			if (!parentTest)
			{
				breeding.BeginBreeding(population, fitnessFunction.getAverageFitness(), 0);
				population.clear();
				population = breeding.getNewPopulation();
			}
			fitnessFunction.Clear();			
			popMemTested = 0;


		}
		neuralNetwork.PurgeWeights();
		neuralNetwork.setWeights(population.at(popMemTested));
		testsDone = 0;
	}
	return 0;
}

GMEXPORT double TimePassed(char* level)
{
	if (startTime == NULL)
	{
		startTime = time(0);
	}
	seconds = difftime(startTime + levelTimes.at(levelNum), time(0));
	if (seconds <= 0)
	{
		return 1;
	}
	return 0;
}

GMEXPORT char* CheckCurrentLevel()
{	
	if (nextLevel == true)
	{
		if (!parentTest)
		{
			SaveParents();
		}
		pInput.DestroyDescoveries();
		cout << "Swapping level" << endl << endl;
		nextLevel = false;
		levelNum++;
		//do this if we are on the last level which should just be the completion screen e.g. ""
		if (levelNum == levels.size() - 1)
		{
			levelNum = 0;
			experimentNumber++; //preemptive measure incase someone wants to run all the same experiments again.
			return levels.at(levels.size() - 1);
		}
		else
		{
			testName = CheckFitnessTest();
			if (testName != "")
			{
				fitnessFunction.UpdateFitnessTest(testName);
			}
			return levels.at(levelNum);
		}
	}
	else
	{
		return levels.at(levelNum);;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma section "Layout"

GMEXPORT double SetCoolGlasses(double d)
{
	if (d == 1)
	{
		coolGlasses = true;
	}
	else
	{
		coolGlasses = false;
	}
	return 0;
}

GMEXPORT double SetUdjatEye(double d)
{
	if (d == 1)
	{
		udjatEye = true;
	}
	else
	{
		udjatEye = false;
	}
	return 0;
}

// Call this each time a new level is loaded
GMEXPORT double ResetFogForNewLevel()
{
	for (int i = 0; i < 42; i++)
	{
		for (int j = 0; j < 34; j++)
		{
			spmap[i][j] = 1;
			mapFog[i][j] = 1;
			spiderWebs[i][j] = 0;
			mapLiquids[i][j] = 0;
		}
	}
	hasResetMap = 1;
	return 1;
}

GMEXPORT double TerrainWasDestroyed(double x, double y)
{
	spmap[(int)x][(int)y] = 0;
	return 0;
}

GMEXPORT double SpiderWebCreated(double x, double y)
{
	spiderWebs[(int)x][(int)y] += 1;
	return 0;
}

GMEXPORT double SpiderWebWasRemoved(double x, double y)
{
	if (spiderWebs[(int)x][(int)y] > 0)
	{
		spiderWebs[(int)x][(int)y] -= 1;
	}
	return 0;
}

GMEXPORT double DestroyFog(double x, double y)
{
	mapFog[(int)x][(int)y] = 0;
	return 0;
}

GMEXPORT double TerrainIsLadder(double x, double y)
{
	spmap[(int)x][(int)y] = 2;
	return 0;
}

GMEXPORT double TerrainIsGoal(double x, double y)
{
	spmap[(int)x][(int)y] = 3;
	return 0;
}

GMEXPORT double TerrainIsStart(double x, double y)
{
	spmap[(int)x][(int)y] = 4;
	return 0;
}

GMEXPORT double TerrainIsAltar(double x, double y)
{
	spmap[(int)x][(int)y] = 5;
	return 0;
}

GMEXPORT double TerrainIsArrowTrapRight(double x, double y)
{
	spmap[(int)x][(int)y] = 6;
	return 0;
}

GMEXPORT double TerrainIsArrowTrapLeft(double x, double y)
{
	spmap[(int)x][(int)y] = 7;
	return 0;
}

GMEXPORT double TerrainIsShopKeeperArea(double x, double y)
{
	spmap[(int)x][(int)y] = 8;
	return 0;
}

GMEXPORT double TerrainIsIce(double x, double y)
{
	spmap[(int)x][(int)y] = 9;
	return 0;
}

GMEXPORT double TerrainIsSpike(double x, double y)
{
	spmap[(int)x][(int)y] = 10;
	return 0;
}

GMEXPORT double TerrainIsSpearTrap(double x, double y)
{
	spmap[(int)x][(int)y] = 11;
	return 0;
}

GMEXPORT double TerrainIsSwimWater(double x, double y)
{
	mapLiquids[(int)x][(int)y] = 1;
	return 0;
}

GMEXPORT double TerrainIsLava(double x, double y)
{
	mapLiquids[(int)x][(int)y] = 2;
	return 0;
}

GMEXPORT double TerrainIsGameEntrance(double x, double y)
{
	spmap[(int)x][(int)y] = 12;
	return 0;
}

GMEXPORT double TerrainIsTree(double x, double y)
{
	spmap[(int)x][(int)y] = 13;
	return 0;
}

GMEXPORT double TerrainIsTreeBranchLeaf(double x, double y)
{
	spmap[(int)x][(int)y] = 14;
	return 0;
}

GMEXPORT double TerrainIsEmpty(double x, double y)
{
	mapLiquids[(int)x][(int)y] = 0;
	return 0;
}

GMEXPORT double GetLiquidValue(double x, double y)
{
	return mapLiquids[(int)x][(int)y];
}

GMEXPORT double SpringPadAtPosition(double x, double y)
{

	return 0;
}



GMEXPORT double FillShopkeeperArea(double x, double y)
{
	// TODO
	// find each shopkeeper area in the map
	// populate sensibly
	spmap[(int)x][(int)y] = 8;
	return 0;
}

/*
	Call this each time a new level is loaded in a loop in GameMaker
	Setting the state of the block as to whether there is a terrain block
	in that coordinate.

	Should also be called when a terrain block is destroyed by *anything*
*/
GMEXPORT double SetMapCoord(double x, double y, double state)
{
	spmap[(int)x][(int)y] = state;
	return state;
}

/// Call this each time a new square is discovered in the map
GMEXPORT double ClearFogFromSquare(double x, double y)
{
	mapFog[(int)x][(int)y] = 0;
	return 0;
}

// returns whether a node has terrain or not
// if the section has not been discovered then it returns false
GMEXPORT double GetNodeState(double x, double y)
{
	if (mapFog[(int)x][(int)y] == 0)
	{
		return spmap[(int)x][(int)y];
	}
	return -1;
}

GMEXPORT double GetFogState(double x, double y)
{
	return mapFog[(int)x][(int)y];
}


GMEXPORT bool IsClearPathToExit()
{
	return 0;
}


#pragma section "Dynamic Objects"

GMEXPORT double ClearDynamicObjects()
{
	for (int i = 0; i < 42; i++)
	{
		for (int j = 0; j < 34; j++)
		{
			pushBlocks[i][j] = 0;
			bats[i][j] = 0;
		}
	}
	return 0;
}

// push blocks
GMEXPORT double NodeContainsPushBlock(double x, double y)
{
	pushBlocks[(int)x][(int)y] = 1;
	return 0;
}

GMEXPORT double GetNodeContainsPushBlock(double x, double y)
{
	if (mapFog[(int)x][(int)y] == 0)
	{
		return pushBlocks[(int)x][(int)y];
	}
	return 0;
}

// enemies

GMEXPORT double NodeContainsBat(double x, double y)
{
	bats[(int)x][(int)y] += 1;
	return 0;
}

// Returns the number of bats for a given node.
GMEXPORT double GetNodeContainsBat(double x, double y)
{
	if (mapFog[(int)x][(int)y] == 0)
	{
		return bats[(int)x][(int)y];
	}
	return 0;
}

#pragma section "Collectables"

GMEXPORT double ResetCollectables()
{
	collectablesList.clear();
	return 0;
}

GMEXPORT double NodeContainsCollectable(double x, double y, double type, double id)
{
	collectableObject object;
	object.x = x;
	object.y = y;
	object.type = type;
	object.id = id;
	collectablesList.push_back(object);
	return 0;
}

GMEXPORT double UpdateCollectableAtNode(double x, double y, double id)
{
	int cSize = collectablesList.size();
	for(int i = 0; i < cSize; i++)
	{
		if (collectablesList.at(i).id == id)
		{
			if (spmap[(int)x][(int)y] == 0 || coolGlasses || udjatEye)
			{
				collectablesList.at(i).x = x;
				collectablesList.at(i).y = y;
			}
			return 0;
		}
	}
	return 0;
}

GMEXPORT double RemoveCollectableWithID(double id)
{
	int cSize = collectablesList.size();
	for(int i = 0; i < cSize; i++)
	{
		if (collectablesList.at(i).id == id)
		{
			collectablesList.erase(collectablesList.begin() + i);
			return 0;
		}
	}
	return 0;
}

GMEXPORT double NumberOfCollectableTypeInNode(double type, double x, double y)
{
	int cSize = collectablesList.size();
	double count = 0;
	if (mapFog[(int)x][(int)y] == 0)
	{
		for(int i = 0; i < cSize; i++)
		{
			if (collectablesList.at(i).type == type)
			{ 
				if ((int)collectablesList.at(i).x == (int) x && (int) collectablesList.at(i).y == (int) y)
				{
					count += 1;
				}
			}
		}
	}
	return count;
}

#pragma section "Enemies"

GMEXPORT double ResetEnemies()
{
	enemiesList.clear();
	return 0;
}

GMEXPORT double NodeContainsEnemy(double x, double y, double type, double id)
{
	collectableObject object;
	object.x = x;
	object.y = y;
	object.type = type;
	object.id = id;
	enemiesList.push_back(object);
	return 0;
}

GMEXPORT double UpdateEnemyAtNode(double x, double y, double id)
{
	int cSize = enemiesList.size();
	for(int i = 0; i < cSize; i++)
	{
		if (enemiesList.at(i).id == id)
		{
			enemiesList.at(i).x = x;
			enemiesList.at(i).y = y;
			return 0;
		}
	}
	return 0;
}

GMEXPORT double RemoveEnemyWithID(double id)
{
	int cSize = enemiesList.size();
	for(int i = 0; i < cSize; i++)
	{
		if (enemiesList.at(i).id == id)
		{
			enemiesList.erase(enemiesList.begin() + i);
			return 0;
		}
	}
	return 0;
}

GMEXPORT double NumberOfWebsInNode(double x, double y)
{
	if (mapFog[(int) x][(int) y] == 0)
	{
		return spiderWebs[(int)x][(int) y];
	}
	return 0;
}

GMEXPORT double NumberOfEnemyTypeInNode(double type, double x, double y)
{
	int cSize = enemiesList.size();
	double count = 0;
	if (mapFog[(int)x][(int)y] == 0)
	{
		for(int i = 0; i < cSize; i++)
		{				
			if (enemiesList.at(i).type == type)
				{ 
					if ((int)enemiesList.at(i).x >= screenX && (int)enemiesList.at(i).x <= screenX + screenW && 
						(int)enemiesList.at(i).y >= screenY && (int)enemiesList.at(i).y <= screenY + screenW)
					{
						if ((int)enemiesList.at(i).x == (int) x && (int) enemiesList.at(i).y == (int) y)
						{
							count += 1;
						}
					}
				}
			}
		}
	return count;
}

GMEXPORT double GetIDOfEnemyInNode(double type, double x, double y)
{
	int cSize = enemiesList.size();
	double count = 0;
	if (mapFog[(int)x][(int)y] == 0)
	{
		for(int i = 0; i < cSize; i++)
		{
			if (enemiesList.at(i).type == type)
			{ 
				if ((int)enemiesList.at(i).x == (int) x && (int) enemiesList.at(i).y == (int) y)
				{
					return enemiesList.at(i).id;
				}
			}
		}
	}
	return 0;
}

GMEXPORT double GetIDOfCollectableInNode(double type, double x, double y)
{
	int cSize = collectablesList.size();
	double count = 0;
	if (mapFog[(int)x][(int)y] == 0)
	{
		for(int i = 0; i < cSize; i++)
		{
			if (collectablesList.at(i).type == type)
			{ 
				if ((int)collectablesList.at(i).x == (int) x && (int) collectablesList.at(i).y == (int) y)
				{
					return collectablesList.at(i).id;
				}
			}
		}
	}
	return 0;
}

#pragma section "Debug"

GMEXPORT double SaveDynamicObjectFilesDebug()
{
	ofstream fileStream;
	fileStream.open("level_layout.txt");
	for (int i = 0; i < 34; i++)
	{
		for (int j = 0; j < 42; j++)
		{
			if (mapFog[j][i] == 0)
			{
				fileStream << spmap[j][i];					
			}
			else
			{
				fileStream << 0;
			}
			fileStream << " ";
		}
		fileStream <<  "\n";
	}
	fileStream.close();
	fileStream.open("level_bats.txt");
	for (int i = 0; i < 34; i++)
	{
		for (int j = 0; j < 42; j++)
		{
			if (mapFog[j][i] == 0)
			{
				fileStream << bats[j][i];					
			}
			else
			{
				fileStream << 0;
			}
			fileStream << " ";
		}
		fileStream <<  "\n";
	}
	fileStream.close();
	fileStream.open("level_liquids.txt");
	for (int i = 0; i < 34; i++)
	{
		for (int j = 0; j < 42; j++)
		{
			fileStream << mapLiquids[j][i];				

			fileStream << " ";
		}
		fileStream <<  "\n";
	}
	fileStream.close();
		fileStream.open("level_collectables.txt");
		int size = collectablesList.size();
		for (int i = 0; i < size; i++)
		{
			fileStream << "TYPE: ";	
			fileStream <<  collectablesList.at(i).type;
			fileStream << " X: ";
			fileStream << collectablesList.at(i).x;
			fileStream << " Y: ";
			fileStream << collectablesList.at(i).y;
			fileStream << " ID: ";
			fileStream << collectablesList.at(i).id;
			fileStream <<  "\n";
		}
	fileStream.close();
	fileStream.open("level_enemies.txt");
	size = enemiesList.size();
		for (int i = 0; i < size; i++)
		{
			fileStream << "TYPE: ";	
			fileStream <<  enemiesList.at(i).type;
			fileStream << " X: ";
			fileStream << enemiesList.at(i).x;
			fileStream << " Y: ";
			fileStream << enemiesList.at(i).y;
			fileStream << " ID: ";
			fileStream << enemiesList.at(i).id;
			fileStream <<  "\n";
		}
	fileStream.close();
	return 0;
}

GMEXPORT double SaveSpiderwebsToFile()
{
	ofstream fileStream;
	fileStream.open("level_webs.txt");
	for (int i = 0; i < 34; i++)
	{
		for (int j = 0; j < 42; j++)
		{
			fileStream << spiderWebs[j][i];	
			fileStream << " ";
		}
		fileStream <<  "\n";
	}
	fileStream.close();
	return 0;
}

GMEXPORT double SaveLevelLayoutToFile()
{
	ofstream fileStream;
	fileStream.open("level_layout.txt");
	for (int i = 0; i < 34; i++)
	{
		for (int j = 0; j < 42; j++)
		{
			if (mapFog[j][i] == 0)
			{
				fileStream << spmap[j][i];					
			}
			else
			{
				fileStream << mapFog[j][i];
			}
			fileStream << " ";
		}
		fileStream <<  "\n";
	}
	fileStream.close();
	return 0;
}



class MapSearchNode
{
public:
	int x;
	int y;
	int gScore;
	int hScore;
	int fScore;
	MapSearchNode *parent;
	bool opened;
	bool closed;
	
	double GetGScore(MapSearchNode *p) 
	{
		return p->gScore + ((x ==p->x || y == p->y) ? 1 : 1.5);
	}

	double GetHScore(MapSearchNode *p)
	{
		/*
		float xDist = ((x) - (p->x));
		float yDist = ((y) - (p->y));
		float distance = sqrt((xDist * xDist) + (yDist * yDist));
		hScore = (double) distance;*/
		return (abs(p->x - x) + abs(p->y - y));
	}

	double GetFScore()
	{
		return fScore;
	}

	void ComputeScores(MapSearchNode * end)
	{
		gScore = GetGScore(parent);
		hScore = GetHScore(end);
		fScore = gScore + hScore;
	}
};

struct nodeValue {
  MapSearchNode node;
  double value;
} ;

MapSearchNode goal;

// A-Star pathfinding.
// heavily based upon http://www.raywenderlich.com/4946/introduction-to-a-pathfinding

std::vector<MapSearchNode*> m_PathList;
std::vector<MapSearchNode*> openList;



// http://xpac27.github.io/a-star-pathfinder-c++-implementation.html

GMEXPORT double CalculatePathFromXYtoXY(double x1, double y1, double x2, double y2)
{

	if (x1 != x2 || y1 != y2)
	{
	m_PathList.clear();

	std::map<int, std::map<int, MapSearchNode*> > grid;
	for (int i = 0; i < 42; i++)
	{
		for (int j = 0; j < 34; j++)
		{
			grid[i][j] = new MapSearchNode();
			grid[i][j]->x = i;
			grid[i][j]->y = j;
		}
	}

	ofstream fileStream;
	fileStream.open("level_paths.txt");
	// define the new nodes
	MapSearchNode* start = new MapSearchNode();
	start->x = x1;
	start->y = y1;

	fileStream << "START";
	fileStream << " START X: ";
	fileStream << start->x;
	fileStream << " START Y: ";
	fileStream << start->y;

	MapSearchNode* end = new MapSearchNode();
	end->x = x2;
	end->y = y2;

	fileStream << "END";
	fileStream << " END X: ";
	fileStream << end->x;
	fileStream << " END Y: ";
	fileStream << end->y;

	MapSearchNode* current = new MapSearchNode();
	MapSearchNode* child = new MapSearchNode();

	std::list<MapSearchNode*> openList;
	std::list<MapSearchNode*> closedList;
	list<MapSearchNode*>::iterator i;

	unsigned int n = 0;

	openList.push_back(start);
	start->opened = true;

	while (n == 0 || (current != end && n < 50))
	{
		// Look for the smallest f value in the openList
		for (i = openList.begin(); i != openList.end(); i++)
		{
			if (i == openList.begin() || (*i)->GetFScore() <= current->GetFScore())
			{
				current = (*i);
			}
		}

		fileStream << "searching";
		fileStream << " Current X: ";
		fileStream << current->x;
		fileStream << " Current Y: ";
		fileStream << current->y;

		// Stop if we've reached the end
		if (current->x == end->x && current->y == end->y)
		{
			fileStream << "end reached";
			break;
		}

		// Remove the current point from the open list
		openList.remove(current);
		current->opened = false;
		
		// Add the current point from the open list
		closedList.push_back(current);
		current->closed = true;

		// Get all the current adjacent walkable points
		for (int x = -1; x < 2; x++)
		{
			for (int y = -1; y < 2; y++)
			{
				if (x == 0 && y == 0)
				{
					// ignore current node, pass
					continue;
				}

				if (x == 0 || y == 0)
				{

					child = grid[current->x + x][current->y + y];

					// if it's closed or not walkable then pass
					if (child->closed || (spmap[child->x][child->y] != 0 && spmap[child->x][child->y] != 3 && spmap[child->x][child->y] != 4 && spmap[child->x][child->y] != 2 && spmap[child->x][child->y] != 9))
					{
						fileStream << "\n";
						fileStream << "closed or not walkable";
						continue;
					}

					// IF AT A CORNER?
	
					// if it's already in the opened list
					if (child->opened)
					{
						if (child->gScore > child->GetGScore(current))
						{
							child->parent = current;
							child->ComputeScores(end);
						}
					}
					else
					{
						openList.push_back(child);
						child->opened = true;
	
						// COMPUTE THE G
						child->parent = current;
						child->ComputeScores(end);
					}
				}
 			}
		}
		n++;
		fileStream << "\n";
	}

	// Reset
	for (i = openList.begin(); i != openList.end(); i++)
	{
		(*i)->opened = false;
	}
	for (i = closedList.begin(); i != closedList.end(); i++)
	{
		(*i)->closed = false;
	}
	fileStream.close();

	fileStream.open("level_path.txt");
	// resolve the path starting from the end point
	while (current->parent && current != start)
	{
		fileStream << "X ";
		fileStream << current->x;
		fileStream << " Y ";
		fileStream << current->y;
		fileStream << "\n";
		m_PathList.push_back(current);
		current = current->parent;
		n ++;
	}
	fileStream.close();
	return 0;
	}
	return 0;
}

GMEXPORT double GetNextPathXPos(double x, double y)
{
	ofstream fileStream;
	fileStream.open("distance.txt");
	float smallestDistance = 0;
	int pos = 0;
	for (int i = 0; i < m_PathList.size(); i++)
	{
		float xDist = ((x) - (m_PathList.at(i)->x));
		float yDist = ((y) - (m_PathList.at(i)->y));
		float distance = sqrt((xDist * xDist) + (yDist * yDist));
		fileStream << distance;
		if (distance < smallestDistance || i == 0)
		{
			if (distance > 0)
			{
				smallestDistance = distance;
				
				pos = i;
			}
		}
	}
	fileStream.close();
	if (pos > 0)
		pos --;
	if (m_PathList.size() > 0)
	return m_PathList.at(pos)->x;
	return x;
}

GMEXPORT double GetNextPathYPos(double x, double y)
{
	float smallestDistance = 0;
	int pos = 0;
	for (int i = 0; i < m_PathList.size(); i++)
	{
		float xDist = ((x) - (m_PathList.at(i)->x));
		float yDist = ((y) - (m_PathList.at(i)->y));
		float distance = sqrt((xDist * xDist) + (yDist * yDist));
		if (distance < smallestDistance || i == 0)
		{
			if (distance > 0)
			{
				smallestDistance = distance;
				pos = i;
			}
		}
	}
	if (pos > 0)
		pos --;
	if (m_PathList.size() > 0)
	return m_PathList.at(pos)->y;
	return y;
}


