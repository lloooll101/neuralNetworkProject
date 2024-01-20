#pragma once
#include <vector>

enum class WarpSize {
	WIDTH_8, WIDTH_16, WIDTH_32, WIDTH_64
};

struct glNeuralNetworkGroup
{
	struct {
		bool AllowNetworkResizeKernelOptimizations; //When enabled, the network configuration will automatically change either up or down to better fit the specified warp size.
		bool UseFP16; 
		bool ForceTighterLayerPacking; //When enabled, resizes networkCount to a multiple of 4 that is guaranteed to be above the old value. If the new value is greater than the max networks allowed in a single group, the create command will fail.
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