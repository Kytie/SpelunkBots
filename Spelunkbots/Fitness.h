#pragma once
#include <windows.h>
#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<sstream>
#include "PlayerInputs.h"

using namespace std;

class Fitness
{
public:
	Fitness();
	~Fitness();
	void CalculateFitness(double completionTimePoints, int maxCompletionTime, double playerPosX, double playerPosY, PlayerInputs* pInputs);
	void UpdateFitnessTest(char* fitnessTest);
	string getFitnessTest();
	void CalculateAverageFitness();
	void Clear();
	vector<double> getAverageFitness();
	int getStats(int currExperimentNumber, int levelNum, bool parentTest);
	void setStartPosition(double x, double y);


private:
	vector<double> fitness;
	vector<double> avgFitness;
	string fitnessTest = "";
	const int mapWidthSqr = 451584; //Total map width squared
	const int mapHeightSqr = 295936; //Total map height squared
	double mapSize = 0;
	double standardDeviation = 0;
	double populationAverage = 0;
	double highestScore = 0;
	ofstream fileStream;
	ostringstream concat;
	string directoryName = "";
	int experimentNumber = 0;
	double startPosX = 0;
	double startPosY = 0;
};

