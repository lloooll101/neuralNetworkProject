#pragma once
#include <vector>

struct glNeuralNetworkGroup
{
	struct {
		bool MaximizeKernelUtilization;
		bool AllowSizeDownscale;
		bool AllowSizeUpscale;
		bool UseFP16;
		bool ForceMultipleOf4;
		bool AllowTighterPacking;
	} flags;

	unsigned int inputWeights;

	std::vector<unsigned int> networkWeights;
	unsigned int networkBiases;

	unsigned int outputWeights;
	unsigned int outputBiases;

	int layers;
	int nodesPerLayer;
	int inputs;
	int outputs;
	int networkCount;
};

bool buildNetworkGroup(glNeuralNetworkGroup&);
bool evalNetworkGroup(glNeuralNetworkGroup&, unsigned int);
bool deleteNetworkGroup(glNeuralNetworkGroup&);

void test();