#include <iostream>

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

	test();

	glNeuralNetworkGroup nn;
	nn.inputs = 5;
	nn.layers = 3;
	nn.nodesPerLayer = 25;
	nn.outputs = 2;
	nn.networkCount = 200;
	buildNetworkGroup(nn);
	deleteNetworkGroup(nn);
	
}