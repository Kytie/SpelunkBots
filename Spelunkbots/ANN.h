#pragma once
#include<iostream>
#include<vector>
#include"Neuron.h"
#include"Chromosome.h"
#include"PlayerInputs.h"
using namespace std;

class ANN
{
public:
	ANN(int isize, int noOfLayers, int hsize, int osize);
	~ANN();
	void setWeights(Chromosome ch);
	void UpdateInputs(PlayerInputs pInputs);
	void Update();
	vector<double> getOutputs();
	void PurgeWeights();

private:
	vector<Neuron> inputs;
	vector<vector<Neuron>> hidden;
	vector<Neuron> outputs;

	int numOfHiddenLayers = 0;
};

