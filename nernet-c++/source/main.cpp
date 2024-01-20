#include <iostream>
#include <vector>

#include "glInit.hpp"
#include "glNetwork.hpp"

//steps:
// create random networks
// solve networks 
// use result to get a score
// create network mask using scores
// erase poor networks and regen with high performing ones
// repeat
//
int main() {
	if (!startOpenGLContext()) return 1;

	//test();

	std::vector<glNeuralNetworkGroup> groups;
	for (int i = 0; i < 2800; i++) {
		glNeuralNetworkGroup nn;
		nn.flags.AllowNetworkResizeKernelOptimizations = false;
		nn.flags.ForceTighterLayerPacking = false;
		nn.flags.UseFP16 = false;
		nn.flags.warpSize = WarpSize::WIDTH_8;
		nn.inputs = 8;
		nn.layers = 2;
		nn.nodesPerLayer = 16;
		nn.outputs = 8;
		nn.networkCount = 1024;
		bool success = buildNetworkGroup(nn);
		if (!success) return 1;
		groups.push_back(nn);
	}
	return 0;
}