#pragma once
#include<iostream>
#include<vector>
#include"Chromosome.h"
#include"Fitness.h"
#include <time.h>

using namespace std;

class BreedingTools
{
public:
	BreedingTools(int maxPopulation, int parentNum, int mutationProb, int crossoverProb, int geneNum);
	~BreedingTools();
	void BeginBreeding(vector<Chromosome> chromosomes, vector<double> fitnessFunction, bool seeded);
	vector<Chromosome> getNewPopulation();

private:
	int maxPopulation;
	int parentNum;
	int mutationProb;
	int crossoverProb;
	int geneNum;
	int childNum;
	vector<Chromosome> oldPop;
	vector<Chromosome> newPop;
	vector<double> fitness;

	void Mutate(Chromosome chromosomes, default_random_engine generator, uniform_real_distribution<double> distribution);
	void Crossover(Chromosome parent1, Chromosome parent2);
	void print();
};

