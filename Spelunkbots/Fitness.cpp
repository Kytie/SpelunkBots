#include "stdafx.h"
#include "Fitness.h"


Fitness::Fitness()
{
	mapSize = sqrt(mapHeightSqr + mapWidthSqr);
}


Fitness::~Fitness()
{
}

vector<double> Fitness::getAverageFitness()
{
	return avgFitness;
}

void Fitness::UpdateFitnessTest(char* fitnessTest)
{
	this->fitnessTest = (string)fitnessTest;
}

string Fitness::getFitnessTest()
{
	return fitnessTest;
}

void Fitness::Clear()
{
	fitness.clear();
	avgFitness.clear();
}

void Fitness::setStartPosition(double x, double y)
{
	startPosX = x-8;
	startPosY = y-8;
}

void Fitness::CalculateFitness(double completionTimePoints, int maxCompletionTime, double playerPosX, double playerPosY, PlayerInputs* pInputs)
{
	if (fitnessTest == "EXIT")
	{
		int noOfDescoveries = pInputs->getDescoveries().size();
		double distance = 0.0;
		//double sqrX = 0;
		//double sqrY = 0;
		double distanceX = 0;
		double distanceY = 0;
		double scoreX = 0;
		double scoreY = 0;
		double fitnessScore = 0;
		vector<double> coords = { 0, 0 };
		//calculate distance between player and the exit (if the bot is aware of its existence).

		if (noOfDescoveries != 0)
		{	
			coords = pInputs->getDescoveryLocation("EXIT");
			//get distance from bot to door at start of level.
			if (coords.at(0) > startPosX)
			{
				distanceX = coords.at(0) - startPosX;
			}
			else if (coords.at(0) < startPosX)
			{
				distanceX = startPosX - coords.at(0);
			}

			if (coords.at(1) > startPosY)
			{
				distanceY = coords.at(1) - startPosY;
			}
			else if (coords.at(1) < startPosY)
			{
				distanceY = startPosY - coords.at(1);
			}

			//work out how far the bot got along the x and y axis to the door.
			if (playerPosX < coords.at(0))
			{
				scoreX = coords.at(0) - playerPosX; //distance will be bigger than playerPosX
			}
			else if (playerPosX > coords.at(0))
			{
				scoreX = playerPosX - coords.at(0); //distance will be smaller than playerPosX
			}
			else
			{
				scoreX = 0; //distance will be the same than playerPosX
			}

			if (playerPosY < coords.at(1))
			{
				scoreY = coords.at(1) - playerPosY; //distance will be bigger than playerPosY
			}
			else if (playerPosY > coords.at(1))
			{
				scoreY = playerPosY - coords.at(1); //distance will be smaller than playerPosY
			}
			else
			{
				scoreY = 0; //distance will be the same than playerPosY
			}

			//work out score
			if (distanceX == 0)
			{
				distanceX = 1;
			}
			scoreX = scoreX / distanceX;

			if (distanceY == 0)
			{
				distanceY = 1;
			}
			scoreY = scoreY / distanceY;

			fitnessScore = 100 - ((scoreX + scoreY / 2)*100);

			// the more time you have left over from comlpeting the level the more points you get.
			// you can have a maximum of 1 point removed from your score, find the percentage of time you saved and take the difference away from 1 e.g. i saved 0% time so the difference between 0 and 1 is 1.
			// I saved 25% time, the difference between 1 and 0.25 is 0.75.
			if (maxCompletionTime == 0)
			{
				maxCompletionTime = 1;
			}
			fitnessScore = fitnessScore - (1.0 - (completionTimePoints / maxCompletionTime));
			if (fitnessScore < 0)
			{
				fitnessScore = 0;
			}
			fitness.push_back(fitnessScore);
		}
		else
		{
			cout << "Could Not Find Exit" << endl;
		}
	}
}

void Fitness::CalculateAverageFitness()
{
	int size = fitness.size();
	double averageFitness = 0;

	for (int i = 0; i < size; i++)
	{
		averageFitness += fitness.at(i);
	}
	averageFitness = (averageFitness / size);
	avgFitness.push_back(averageFitness);
	fitness.clear();
}

int Fitness::getStats(int currExperimentNumber, int levelNum, bool parentTest)
{
	if (experimentNumber != currExperimentNumber)//if the stored experiment number is not equal to the experiment number that can been passed in.
	{
		experimentNumber = currExperimentNumber;
		concat.str("");
		concat << "..\\..\\GenerationStats\\Experiment-" << experimentNumber;
		directoryName = concat.str();
		if (!CreateDirectoryA(directoryName.c_str(), NULL))
		{
			if (ERROR_PATH_NOT_FOUND == GetLastError())
			{
				cout << "Unable to Create Experiment Directory, Please Check Filepath - from spelunky_1_1 folder it should be ..\\..\\GenerationStats\\Experiment-X." << endl;
			}
			else
			{
				if (ERROR_ALREADY_EXISTS == GetLastError()) //if folder already exists.
				{
					bool done = 0;
					while (!done)
					{
						concat.str("");
						experimentNumber++;
						concat << "..\\..\\GenerationStats\\Experiment-" << experimentNumber;
						directoryName = concat.str();
						if (CreateDirectoryA(directoryName.c_str(), NULL))
						{
							done = 1;
						}
						else if (ERROR_PATH_NOT_FOUND == GetLastError())
						{
							cout << "Unable to Create Experiment Directory, Please Check Filepath - from spelunky_1_1 folder it should be ..\\..\\GenerationStats\\Experiment-X." << endl;
						}
					}
				}
			}
		}
	}

	if (!parentTest)
	{
		//Calculate standard deviation
		for (int i = 0; i < avgFitness.size(); i++)
		{
			populationAverage = populationAverage + avgFitness.at(i);
			if (avgFitness.at(i) > highestScore)
			{
				highestScore = avgFitness.at(i);
			}
		}
		populationAverage = populationAverage / avgFitness.size(); // This gives us the mean fitness score.

		for (int j = 0; j < avgFitness.size(); j++)
		{
			standardDeviation = standardDeviation + pow(avgFitness.at(j) - populationAverage, 2);
		}
		standardDeviation = standardDeviation / avgFitness.size(); //This gets us the average distance from the mean squared.
		standardDeviation = sqrt(standardDeviation); //This gives us the standard deviation


		//Send standard deviation information to file for import into Excel spreadsheet.

		concat.str("");
		concat << "..\\..\\GenerationStats\\Experiment-" << experimentNumber << "\\GenerationsData-Level-" << levelNum << ".txt";
		directoryName = concat.str();
		fileStream.open(directoryName.c_str(), ofstream::app);
		if (fileStream.is_open())
		{
			concat.str("");
			concat << populationAverage << "," << standardDeviation << "," << highestScore;
			fileStream << concat.str() << endl;
		}
		else
		{
			cout << "Unable to write Generation Statistics data to file." << endl;
		}

		fileStream.close();
		standardDeviation = 0;
		populationAverage = 0;
		highestScore = 0;
	}
	else
	{
		concat.str("");
		concat << "..\\..\\GenerationStats\\Experiment-" << experimentNumber << "\\ParentsData-Level-" << levelNum << ".txt";
		directoryName = concat.str();
		fileStream.open(directoryName.c_str(), ofstream::app);
		if (fileStream.is_open())
		{
			for (int k = 0; k < avgFitness.size(); k++)
			{
				concat.str("");
				concat << avgFitness.at(k);
				fileStream << concat.str() << endl;
			}
		}
		else
		{
			cout << "Unable to write Parent data to file." << endl;
		}
		fileStream.close();
	}

	return experimentNumber;
}
