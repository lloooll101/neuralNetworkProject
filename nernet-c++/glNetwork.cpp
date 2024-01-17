#include "glNetwork.hpp"
#include <glad/gl.h>

bool buildNetwork(glNeuralNetwork& network) {
	if (network.layers <= 1 || network.nodesPerLayer == 0 || network.inputs == 0 || network.outputs == 0) {
		return false;
	}

	glGenTextures(1, &(network.inputWeights));
	glGenTextures(1, &(network.networkWeights));
	glGenTextures(1, &(network.networkBiases));
	glGenTextures(1, &(network.outputWeights));
	glGenTextures(1, &(network.outputBiases));

	//input weights
	glBindTexture(GL_TEXTURE_2D, network.inputWeights);
	//dont interpolate
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, network.inputs, network.nodesPerLayer, 0, GL_RED, GL_FLOAT, nullptr);

	//network weights
	glBindTexture(GL_TEXTURE_2D_ARRAY, network.networkWeights);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, network.nodesPerLayer, network.nodesPerLayer, network.layers - 1, 0, GL_RED, GL_FLOAT, nullptr);

	//network biases
	glBindTexture(GL_TEXTURE_1D_ARRAY, network.networkBiases);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_R32F, network.nodesPerLayer, network.layers, 0, GL_RED, GL_FLOAT, nullptr);

	//output weights
	glBindTexture(GL_TEXTURE_2D, network.outputWeights);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, network.nodesPerLayer, network.outputs, 0, GL_RED, GL_FLOAT, nullptr);

	//output biases
	glBindTexture(GL_TEXTURE_1D, network.outputBiases);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, network.outputs, 0, GL_RED, GL_FLOAT, nullptr);


}
bool evalNetwork(glNeuralNetwork& network, unsigned int inpBuf) {

}
bool deleteNetwork(glNeuralNetwork& network) {

}