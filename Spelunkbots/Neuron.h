#pragma once
#include<iostream>
#include<vector>
using namespace std;

class Neuron
{
public:
	Neuron();
	~Neuron();

	void setConnectedNeurons(Neuron *neuron);
	double getWeight(int index);
	void setWeight(double weight);
	void setValue(double value);
	double getValue();
	int getNumOfConnectedNeurons();
	double getConnectedNeuronValue(int index);
	void PurgeWeights();

private:
	vector<Neuron*> connectedNeurons;
	vector<double> weights;
	double value;
};