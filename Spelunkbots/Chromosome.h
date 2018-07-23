#pragma once
#include<iostream>
#include<vector>
#include <random>
using namespace std;

class Chromosome
{
public:
	Chromosome(int size);
	Chromosome(const Chromosome& other);
	Chromosome(vector<double> genes);
	~Chromosome();

	double getGene(int index) const;
	int getSize() const;
	void setGene(int index, double gene);
	void CopyAllGenes(vector<double> other);
	void CreateGenes();
	

private:
	vector<double> genes;
};

