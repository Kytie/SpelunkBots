#include "stdafx.h"
#include "Neuron.h"


Neuron::Neuron()
{
}


Neuron::~Neuron()
{
}

void Neuron::setConnectedNeurons(Neuron *neuron)
{
	connectedNeurons.push_back(neuron);
}

double Neuron::getWeight(int index)
{
	return weights.at(index);
}

void Neuron::setWeight(double weight)
{
	weights.push_back(weight);
}

void Neuron::PurgeWeights()
{
	weights.clear();
}

void Neuron::setValue(double value)
{
	this->value = value;
}

double Neuron::getValue()
{
	return value;
}

int Neuron::getNumOfConnectedNeurons()
{
	return connectedNeurons.size();
}

double Neuron::getConnectedNeuronValue(int index)
{
	return connectedNeurons.at(index)->getValue();
}