#include <iostream>

#include "glInit.hpp"
#include "glNetwork.hpp"

int main() {
	if (!startOpenGLContext()) return 1;

	glNeuralNetwork nn;
	nn.inputs = 5;
	nn.layers = 2;
	nn.nodesPerLayer = 5;
	nn.outputs = 2;

	buildNetwork(nn);
}