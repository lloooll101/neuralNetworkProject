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

	glNeuralNetworkGroup nn;
	nn.flags.UseLargeKernels = false;
	nn.flags.UseFP16 = false;
	nn.inputs = 16;
	nn.layers = 4;
	nn.nodesPerLayer = 64;
	nn.outputs = 4;
	nn.networkCount = 1024;
	bool success = buildNetworkGroup(nn);

	success = evalNetworkGroup(nn, 0, 0);

	//test();

	/*std::vector<glNeuralNetworkGroup> groups;
	for (int i = 0; i < 512; i++) {
		glNeuralNetworkGroup nn;
		nn.flags.UseLargeKernels = true;
		nn.flags.UseFP16 = true;
		nn.inputs = 8;
		nn.layers = 2;
		nn.nodesPerLayer = 64;
		nn.outputs = 8;
		nn.networkCount = 2048;
		bool success = buildNetworkGroup(nn);
		if (!success) return 1;
		groups.push_back(nn);
	}*/
	return 0;
}