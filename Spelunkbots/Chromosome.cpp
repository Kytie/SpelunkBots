#include "stdafx.h"
#include "Chromosome.h"

Chromosome::Chromosome(int size)
{
	for (int i = 0; i < size; i++)
	{
		genes.push_back(0.0);
	}
}

Chromosome::Chromosome(const Chromosome& other)
{
	int size = other.getSize();
	for (int i = 0; i < size; i++)
	{
		genes.push_back(other.getGene(i));
	}
}

Chromosome::Chromosome(vector<double> otherGenes)
{
	genes.swap(otherGenes);
}

Chromosome::~Chromosome()
{
}

void Chromosome::CreateGenes()
{
	int chromosomeSize = genes.size();
	random_device rd;
	default_random_engine generator(rd());
	uniform_real_distribution<double> distribution(-1, 1);

	for (int i = 0; i < chromosomeSize; i++)
	{
		genes.at(i) = distribution(generator);
	}
}

double Chromosome::getGene(int index) const
{
	return genes.at(index);
}

int Chromosome::getSize() const
{
	return genes.size();
}

void Chromosome::setGene(int index, double gene)
{
	genes.at(index) = gene;
}

void Chromosome::CopyAllGenes(vector<double> other)
{
	genes = other;
}