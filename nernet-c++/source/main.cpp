#include <iostream>
#include <vector>
#include <chrono>

#include "glInit.hpp"
#include "glNetwork.hpp"
#include "glBufferUtils.hpp";


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


	//things to add:
	//more feed forward kernels
	//special optimized ff kernel
	//compute shader pong game
	//1v1 pong game?
	//network mutate kernels		
	//

	glNeuralNetworkGroup nn;
	nn.flags.UseLargeKernels = false;
	nn.flags.UseFP16 = false;
	nn.inputs = 4;
	nn.layers = 2;
	nn.nodesPerLayer = 4;
	nn.outputs = 4;
	nn.networkCount = 2048;
	bool success = buildNetworkGroup(nn);


	unsigned int input = Create1DArrayBuffer(nn.inputs, nn.networkCount, false);
	unsigned int output = Create1DArrayBuffer(nn.outputs, nn.networkCount, false);
	Fill1DArrayRandom(input, nn.inputs, nn.networkCount, false);

	auto time = std::chrono::high_resolution_clock::now();
	unsigned int evalsps = 0;
	while (evalNetworkGroup(nn, input, output)) {
		unsigned int temp = output;
		output = input;
		input = temp;
		evalsps++;
		auto dt = std::chrono::high_resolution_clock::now() - time;
		if (dt > std::chrono::seconds(1)) {
			std::cout << "Network evaluations p/s: " << evalsps * nn.networkCount << "\n";
			time = std::chrono::high_resolution_clock::now();
			evalsps = 0;
		}
	}
	

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