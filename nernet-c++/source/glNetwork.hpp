#pragma once
#include <vector>

enum class WarpSize {
	WIDTH_8, WIDTH_16, WIDTH_32, WIDTH_64
};

struct glNeuralNetworkGroup
{
	struct {
		bool AutoOptimizeWarpSize;
		bool AllowNetworkResizeKernelOptimizations;
		bool UseFP16;
		bool ForceTighterLayerPacking;
		WarpSize warpSize;
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
bool evalNetworkGroup(glNeuralNetworkGroup&, unsigned int, unsigned int);
bool deleteNetworkGroup(glNeuralNetworkGroup&);

void test();