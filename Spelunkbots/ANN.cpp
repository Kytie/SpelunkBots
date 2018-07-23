#include "stdafx.h"
#include "ANN.h"


ANN::ANN(int isize, int noOfHLayers, int hsize, int osize)
{
	numOfHiddenLayers = noOfHLayers;
	Neuron neuron;

	//setup the neuron layers
	for (int i = 0; i < isize; i++)
	{

		inputs.push_back(neuron);
	}

	for (int i = 0; i < noOfHLayers; i++)
	{
		vector<Neuron> hiddenLayer;
		hidden.push_back(hiddenLayer);
		for (int j = 0; j < hsize; j++)
		{
			hidden.at(i).push_back(neuron);
		}

	}

	for (int i = 0; i < osize; i++)
	{
		outputs.push_back(neuron);
	}

	//connect the nuron layers
	//connect outputs to rightmost hidden layer
	int hlayerNo = numOfHiddenLayers - 1;

	for (int i = 0; i < osize; i++)
	{
		for (int j = 0; j < hsize; j++)
		{
			outputs.at(i).setConnectedNeurons(&hidden.at(hlayerNo).at(j));
		}
	}

	//connect hidden layers together from right to left (unless there is only one hidden layer).
	for (int i = hlayerNo; i > 0; i--)
	{
		for (int j = 0; j < hsize; j++)
		{
			for (int k = 0; k < hsize; k++)
			{
				hidden.at(hlayerNo).at(j).setConnectedNeurons(&hidden.at(hlayerNo - 1).at(k));
			}
		}
		hlayerNo--;
	}

	//connect leftmost hidden layer to the inputs.
	for (int i = 0; i < hsize; i++)
	{
		for (int j = 0; j < isize; j++)
		{
			hidden.at(hlayerNo).at(i).setConnectedNeurons(&inputs.at(j));
		}
	}
}


ANN::~ANN()
{
}

void ANN::setWeights(Chromosome cj)
{
	int geneIndex = 0;

	for (int i = 0; i < outputs.size(); i++)
	{
		for (int j = 0; j < outputs.at(i).getNumOfConnectedNeurons(); j++)
		{
			outputs.at(j).setWeight(cj.getGene(geneIndex));
			geneIndex++;
		}
	}

	for (int i = numOfHiddenLayers - 1; i > -1; i--)
	{
		for (int j = 0; j < hidden.at(i).size(); j++)
		{
			for (int k = 0; k < hidden.at(i).at(j).getNumOfConnectedNeurons(); k++)
			{
				hidden.at(i).at(j).setWeight(cj.getGene(geneIndex));
				geneIndex++;
			}
		}
	}
}

void ANN::PurgeWeights()
{
	for (int i = 0; i < hidden.size(); i++)
	{
		for (int j = 0; j < hidden.at(i).size(); j++)
		{
			hidden.at(i).at(j).PurgeWeights();
		}
	}

	for (int i = 0; i < outputs.size(); i++)
	{
		outputs.at(i).PurgeWeights();
	}
}

void ANN::UpdateInputs(PlayerInputs pInputs)
{
	for (int i = 0; i < inputs.size(); i++)
	{
		switch (i)
		{
		case 0:
			inputs.at(i).setValue(pInputs.DistToDescovery());
			break;
		case 1:
			inputs.at(i).setValue(pInputs.DirectionOfDescovery());
			break;
		case 2:
			inputs.at(i).setValue(pInputs.HeightOfDescovery());
			break;
		case 3:
			inputs.at(i).setValue(pInputs.CheckForObstacle());
			break;
		}
	}
}

void ANN::Update()
{
	double value = 0.0;

	for (int i = 0; i < numOfHiddenLayers; i++) //iterates through the number of hidden layers
	{
		for (int j = 0; j < hidden.at(i).size(); j++) //iterates through the neurons in the above hidden layer index
		{
			for (int k = 0; k < hidden.at(i).at(j).getNumOfConnectedNeurons(); k++) //iterates through the neurons connected to this neuron
			{
				value = value + hidden.at(i).at(j).getWeight(k)*hidden.at(i).at(j).getConnectedNeuronValue(k);
			}

			//use tanH transfer function
			value = ((exp(value) - exp(-value)) / 2) / ((exp(value) + exp(-value) / 2));
			hidden.at(i).at(j).setValue(value);
			value = 0.0; // reset value
		}
	}

	for (int i = 0; i < outputs.size(); i++)// iterates through number of outputs
	{
		for (int j = 0; j < outputs.at(i).getNumOfConnectedNeurons(); j++) //iterates through the neurons connected to this neuron.
		{
			value = value + outputs.at(i).getWeight(j)*outputs.at(i).getConnectedNeuronValue(j);
		}

		//use tanH transfer function
		value = ((exp(value) - exp(-value)) / 2) / ((exp(value) + exp(-value) / 2));
		outputs.at(i).setValue(value);
		value = 0.0; // reset value
	}
}

vector<double> ANN::getOutputs()
{
	vector<double> outputValues;
	for (int i = 0; i < outputs.size(); i++)
	{
		outputValues.push_back(outputs.at(i).getValue());
	}
	return outputValues;
}
