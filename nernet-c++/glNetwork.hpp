#pragma once

struct glNeuralNetwork
{
	unsigned int inputWeights = 0;
	unsigned int networkWeights = 0;
	unsigned int networkBiases = 0;
	unsigned int outputWeights = 0;
	unsigned int outputBiases = 0;

	int layers = 0;
	int nodesPerLayer = 0;
	int inputs = 0;
	int outputs = 0;
};

bool buildNetwork(glNeuralNetwork&);
bool evalNetwork(glNeuralNetwork&, unsigned int);
bool deleteNetwork(glNeuralNetwork&);