#include "stdafx.h"
#include "BreedingTools.h"


BreedingTools::BreedingTools(int maxPop, int pNum, int mProb, int cProb, int gNum)
{
	maxPopulation = maxPop;
	parentNum = pNum;
	mutationProb = mProb;
	crossoverProb = cProb;
	geneNum = gNum;
	if ((maxPopulation % parentNum) == 0)
	{
		childNum = (maxPopulation / parentNum) - 1;
	}
	else
	{
		childNum = 0;
	}
	srand(time(NULL));
}

BreedingTools::~BreedingTools()
{
}

vector<Chromosome> BreedingTools::getNewPopulation()
{
	return newPop;
}

void BreedingTools::BeginBreeding(vector<Chromosome> chromosomes, vector<double> tempfitness, bool seeded)
{
	if (childNum == 0)
	{
		cout << "If not parent testing, please change parentNum to a number that can be devided into maxPopulation without leaving a remainder and is more than 0." << endl << endl;
		newPop = chromosomes;
	}
	else
	{
		oldPop = chromosomes;
		fitness = tempfitness;
		newPop.clear();

		//sort chromosomes and scores into highest fitness to lowest fitness
		if (!seeded)
		{
			bool swapping = true;
			double score = 0;
			while (swapping)
			{
				swapping = false;
				score = 0;
				for (int i = 0; i < fitness.size(); i++)
				{
					score = fitness.at(i);
					if (i != fitness.size() - 1)
					{
						if (fitness.at(i + 1) > score)
						{
							swapping = true;
							score = fitness.at(i + 1);
							fitness.insert(fitness.begin() + i, fitness.at(i + 1));
							fitness.erase(fitness.begin() + (i + 2));
							oldPop.insert(oldPop.begin() + (i), oldPop.at(i + 1));
							oldPop.erase(oldPop.begin() + (i + 2));
						}
					}
				}
			}
		}
		//add parents to new popualation
		for (int i = 0; i < parentNum; i++)
		{
			newPop.push_back(oldPop.at(i));
		}

		//set up random number generator for mutation
		random_device rd;
		default_random_engine generator(rd());
		uniform_real_distribution<double> distribution(-1, 1);

		//Decide on which breeding method to use.
		for (int i = 0; i < parentNum; i++)
		{
			//breeding methods
			if (rand() % 101 <= crossoverProb)
			{
				if (i + 1 != parentNum) //If there are enough parents left to breed then use crossover breeding, otherwise do mutate.
				{
					Crossover(oldPop.at(i), oldPop.at(i + 1));
					i++;
				}
				else
				{
					Mutate(newPop.at(i), generator, distribution);
				}
			}
			else
			{
				Mutate(newPop.at(i), generator, distribution);
			}
		}
	}
}

void BreedingTools::Mutate(Chromosome parent, default_random_engine generator, uniform_real_distribution<double> distribution)
{
	//create new population member - random genes at this point
	Chromosome newPopMem(parent);
	
	//loop through the population member and test to see whether the gene should stay the same as the parents or should be mutated.
	for (int i = 0; i < childNum; i++)
	{
		for (int j = 0; j < geneNum; j++)
		{
			if (rand() % 101 <= mutationProb)
			{
				newPopMem.setGene(j, distribution(generator));
			}
		}
		newPop.push_back(newPopMem);
	}
}

void BreedingTools::Crossover(Chromosome parent1, Chromosome parent2)
{
	//crossover method is similar to cut and splice crossover but both groups are split in a way that when the genes are exchanged the members retain thier original size e.g.
	//******|**
	//    |- exchange these, don't exchange the remaining genes as then you will have just created a child that is a reordered version of 1 parent and not a mixture of 2 parents.
	//    V
	//**|******

	int splitPoint = 0;
	vector<double> geneHolder1;
	vector<double> geneHolder2;
	Chromosome c1(geneNum);
	Chromosome c2(geneNum);
	int index = 0;
	int size = parent1.getSize(); //all chromosomes should have the same number of genes

	//the -1 and +1 constrain the rand to choose a number between 1 - 18 (if chromosomeNum = 20),
	//this way we get at least 1 gene crossing over and also not just a straight swap heppening.
	for (int i = 0; i < childNum; i++)
	{
		splitPoint = rand() % (geneNum - 1) + 1;

		//Store the genes up to the split point.
		for (int j = 0; j < splitPoint; j++)
		{
			geneHolder1.push_back(parent1.getGene(j));
			index++;
		}	
		//Store the genes from parent 2 until the geneHolder has the maximum number of genes it can have.
		for (int k = 0; k < (size - splitPoint); k++)
		{
			geneHolder1.push_back(parent2.getGene(k));
		}

		//The leftover genes that have not been taken from either parent will be enough to make a new child.
		//grab the leftover genes in parent 2 and store them.
		for (int l = (size - splitPoint); l < size; l++)
		{
			geneHolder2.push_back(parent2.getGene(l));
		}
		//Grab the rest of the genes in parent 1 and store them.
		for (int p = index; p < size; p++)
		{
			geneHolder2.push_back(parent1.getGene(p));
		}

		//Create Children.
		c1.CopyAllGenes(geneHolder1);
		c2.CopyAllGenes(geneHolder2);
		index = 0;

		//clear genes for next iteration.
		geneHolder1.clear();
		geneHolder2.clear();

		//add children to population.
		newPop.push_back(c1);
		newPop.push_back(c2);
	}
}
