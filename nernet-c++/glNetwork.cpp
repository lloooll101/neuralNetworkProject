#include "glNetwork.hpp"
#include <glad/gl.h>
#include <random>

#include <fstream>
#include <sstream>

bool computeShadersCompiled = false;
unsigned int fillRandom1D_Program;
unsigned int fillRandom1DArr_Program;
unsigned int fillRandom2D_Program;
unsigned int fillRandom2DArr_Program;

bool compileComputeShaders() {
	const char* fillrand1d = "fillrandom1d.comp";
	const char* fillrand1darr = "fillrandom1darr.comp";
	const char* fillrand2d = "fillrandom2d.comp";
	const char* fillrand2darr = "fillrandom2darr.comp";

	std::ifstream stream;
	std::stringstream ss;

	stream.open(fillrand1d);
	if (stream.fail()) return false;
	ss << stream.rdbuf();
	stream.close();
	std::string fillrand1dstr = ss.str();

	stream = std::ifstream();
	ss = std::stringstream();

	stream.open(fillrand1darr);
	if (stream.fail()) return false;
	ss << stream.rdbuf();
	stream.close();
	std::string fillrand1darrstr = ss.str();

	stream = std::ifstream();
	ss = std::stringstream();

	stream.open(fillrand2d);
	if (stream.fail()) return false;
	ss << stream.rdbuf();
	stream.close();
	std::string fillrand2dstr = ss.str();

	stream = std::ifstream();
	ss = std::stringstream();

	stream.open(fillrand2darr);
	if (stream.fail()) return false;
	ss << stream.rdbuf();
	stream.close();
	std::string fillrand2darrstr = ss.str();

	unsigned int computeshader[4];

	const char* cfr1d = fillrand1dstr.c_str();
	const char* cfr1darr = fillrand1darrstr.c_str();
	const char* cfr2d = fillrand2dstr.c_str();
	const char* cfr2darr = fillrand2darrstr.c_str();

	computeshader[0] = glCreateShader(GL_COMPUTE_SHADER);
	computeshader[1] = glCreateShader(GL_COMPUTE_SHADER);
	computeshader[2] = glCreateShader(GL_COMPUTE_SHADER);
	computeshader[3] = glCreateShader(GL_COMPUTE_SHADER);
	
	glShaderSource(computeshader[0], 1, &cfr1d, nullptr);
	glShaderSource(computeshader[1], 1, &cfr1darr, nullptr);
	glShaderSource(computeshader[2], 1, &cfr2d, nullptr);
	glShaderSource(computeshader[3], 1, &cfr2darr, nullptr);

	glCompileShader(computeshader[0]);
	glCompileShader(computeshader[1]);
	glCompileShader(computeshader[2]);
	glCompileShader(computeshader[3]);

	fillRandom1D_Program = glCreateProgram();
	fillRandom1DArr_Program = glCreateProgram();
	fillRandom2D_Program = glCreateProgram();
	fillRandom2DArr_Program = glCreateProgram();

	glAttachShader(fillRandom1D_Program, computeshader[0]);
	glAttachShader(fillRandom1DArr_Program, computeshader[1]);
	glAttachShader(fillRandom2D_Program, computeshader[2]);
	glAttachShader(fillRandom2DArr_Program, computeshader[3]);

	glLinkProgram(fillRandom1D_Program);
	glLinkProgram(fillRandom1DArr_Program);
	glLinkProgram(fillRandom2D_Program);
	glLinkProgram(fillRandom2DArr_Program);

	glDeleteShader(computeshader[0]);
	glDeleteShader(computeshader[1]);
	glDeleteShader(computeshader[2]);
	glDeleteShader(computeshader[3]);

	computeShadersCompiled = true;
	return true;
}

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

	//seed a prng
	unsigned int rngseedtex;
	glGenTextures(1, &rngseedtex);
	glBindTexture(GL_TEXTURE_1D, rngseedtex);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 1, 0, GL_RED, GL_FLOAT, nullptr);

	std::random_device rd;
	float random = (float)rd() / rd.max();

	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 1, GL_RED, GL_FLOAT, (const void*)&random);

	//enable texture units
	glActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE1);

	//check programs
	if (!computeShadersCompiled) {
		if (!compileComputeShaders()) return false;
	}

	//call randomize compute shader (input weights)
	glBindImageTexture(0, network.inputWeights, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindImageTexture(1, rngseedtex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glUseProgram(fillRandom2D_Program);
	glDispatchCompute(network.inputs, network.nodesPerLayer, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	random = (float)rd() / rd.max();
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 1, GL_RED, GL_FLOAT, (const void*)&random);


	//network weights
	glBindImageTexture(0, network.networkWeights, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindImageTexture(1, rngseedtex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glUseProgram(fillRandom2DArr_Program);
	glDispatchCompute(network.nodesPerLayer, network.nodesPerLayer, network.layers);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	random = (float)rd() / rd.max();
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 1, GL_RED, GL_FLOAT, (const void*)&random);

	//network biases
	glBindImageTexture(0, network.networkBiases, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindImageTexture(1, rngseedtex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glUseProgram(fillRandom1DArr_Program);
	glDispatchCompute(network.nodesPerLayer, network.layers, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	random = (float)rd() / rd.max();
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 1, GL_RED, GL_FLOAT, (const void*)&random);

	//output weights
	glBindImageTexture(0, network.outputWeights, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindImageTexture(1, rngseedtex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glUseProgram(fillRandom2D_Program);
	glDispatchCompute(network.nodesPerLayer, network.outputs, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	random = (float)rd() / rd.max();
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 1, GL_RED, GL_FLOAT, (const void*)&random);

	//output biases
	glBindImageTexture(0, network.outputBiases, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindImageTexture(1, rngseedtex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glUseProgram(fillRandom1D_Program);
	glDispatchCompute(network.outputs, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	

	/*glMemoryBarrier(GL_ALL_BARRIER_BITS);
	float* debugarr = new float[network.inputs * network.nodesPerLayer];
	glBindTexture(GL_TEXTURE_2D, network.inputWeights);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, debugarr);*/

	glDeleteTextures(1, &rngseedtex);

	return true;
}
bool evalNetwork(glNeuralNetwork& network, unsigned int inpBuf) {
	return true;
}
bool deleteNetwork(glNeuralNetwork& network) {
	return true;
}