#include "glNetwork.hpp"
#include <glad/gl.h>
#include <random>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>



bool computeShadersCompiled = false;
enum GLComputePrograms {
	FillRandom1DArray, FillRandom2DArray, FillRandom4x4x4_2DArray
};
std::map<GLComputePrograms, unsigned int> ProgramMap;

bool compileComputeShaders() {
	auto AddProgramToMap = [&](const std::string& filename, GLComputePrograms programtype) -> bool {
		std::ifstream stream;
		std::stringstream ss;

		stream.open(filename);
		if (stream.fail()) return false;
		ss << stream.rdbuf();
		stream.close();

		std::string programSrcCode = ss.str();
		const char* c_srccode = programSrcCode.c_str();

		unsigned int shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &c_srccode, NULL);
		glCompileShader(shader);

		int success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {	
			int log_length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
			char* log = new char[log_length + 1];
			log[log_length] = '\0';
			glGetShaderInfoLog(shader, log_length, nullptr, log);
			std::cout << log << std::endl;
			delete[] log;
			return false;
		}

		unsigned int program = glCreateProgram();
		glAttachShader(program, shader);
		glLinkProgram(program);
		
		glDeleteShader(shader);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			int log_length;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
			char* log = new char[log_length + 1];
			log[log_length] = '\0';
			glGetProgramInfoLog(program, log_length, nullptr, log);
			std::cout << log << std::endl;
			delete[] log;
			return false;
		}
		try {
			ProgramMap.insert({ {programtype, program} });
		}
		catch (std::exception) {
			return false;
		}
		
		return true;
	};

	bool success = false;

	do {
		if (!AddProgramToMap("resources/fillrandom1darr.comp", FillRandom1DArray)) break; //optimize fill random kernel
		if (!AddProgramToMap("resources/fillrandom2darr.comp", FillRandom2DArray)) break;
		if (!AddProgramToMap("resources/4x4x4rand2darr.comp", FillRandom4x4x4_2DArray)) break;

		success = true;
	} while (false);
	
	if (!success) {
		return false;
	}
	computeShadersCompiled = true;
	return true;
}

void test() {
	if (!computeShadersCompiled) {
		if (!compileComputeShaders()) {
			return;
		}
	}

	
}
long roundUpDiv(long dividend, long divisor) {
	return (dividend / divisor) + (dividend % divisor != 0);
}

bool buildNetworkGroup(glNeuralNetworkGroup& network) {

	int maxTextureSize, maxTextureLayers, maxWorkGroupInvocations;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTextureLayers);
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxWorkGroupInvocations);

	if (network.layers <= 0 || network.nodesPerLayer <= 0 || network.inputs <= 0 || network.outputs <= 0 || network.networkCount <= 0) {
		std::cout << "Invalid network configuration!" << std::endl;
		return false;
	}

	if (network.networkCount > maxTextureLayers) {
		std::cout << "Too many networks in one group! Consider reducing the amount of networks you are training or split up the networks required into multiple groups." << std::endl;
		return false;
	}
	if (network.inputs > maxTextureSize || network.nodesPerLayer > maxTextureSize || network.layers > maxTextureSize || network.outputs > maxTextureSize) {
		std::cout << "The OpenGL implementation on this device does not support networks of this size. Consider reducing your specified network size, such as layers, nodes per layer, inputs, and outputs." << std::endl;
		return false;
	}

	//extra edge cases
	if (network.flags.AutoOptimizeWarpSize && network.flags.AllowNetworkResizeKernelOptimizations) {
		std::cout << "AutoOptimizeWarpSize and AllowNetworkResizeKernelOptimizations cannot be used together as they rely on each other being constant." << std::endl;
		return false;
	}

	glGenTextures(1, &(network.inputWeights));
	//glGenTextures(1, &(network.networkWeights));
	glGenTextures(1, &(network.networkBiases));
	glGenTextures(1, &(network.outputWeights));
	glGenTextures(1, &(network.outputBiases));

	//input weights
	glBindTexture(GL_TEXTURE_2D_ARRAY, network.inputWeights);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, network.inputs, network.nodesPerLayer, network.networkCount, 0, GL_RED, GL_FLOAT, nullptr);

	//network weights
	for (int i = 0; i < network.layers - 1; i++) {
		unsigned int texbuf;
		glGenTextures(1, &texbuf);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texbuf);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, network.nodesPerLayer, network.nodesPerLayer, network.networkCount, 0, GL_RED, GL_FLOAT, nullptr);
		network.networkWeights.push_back(texbuf);
	}
	/*glBindTexture(GL_TEXTURE_2D_ARRAY, network.networkWeights);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, network.nodesPerLayer, network.nodesPerLayer, (network.layers - 1) * network.networkCount, 0, GL_RED, GL_FLOAT, nullptr);*/

	//network biases
	glBindTexture(GL_TEXTURE_2D_ARRAY, network.networkBiases);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, network.nodesPerLayer, network.layers, network.networkCount, 0, GL_RED, GL_FLOAT, nullptr);

	//output weights
	glBindTexture(GL_TEXTURE_2D_ARRAY, network.outputWeights);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, network.nodesPerLayer, network.outputs, network.networkCount, 0, GL_RED, GL_FLOAT, nullptr);

	//output biases
	glBindTexture(GL_TEXTURE_1D_ARRAY, network.outputBiases);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_R32F, network.outputs, network.networkCount, 0, GL_RED, GL_FLOAT, nullptr);

	//build programs
	if (!computeShadersCompiled) {
		if (!compileComputeShaders()) {
			return false;
		}
	}

	//create random values
	//create true rng for seed gen
	std::random_device trueRng;

	//input weights
	glBindImageTexture(0, network.inputWeights, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUseProgram(ProgramMap[FillRandom4x4x4_2DArray]);
	glUniform1f(0, (float)trueRng() / trueRng.max());
	glUniform3i(1, network.inputs, network.nodesPerLayer, network.networkCount);
	glDispatchCompute(roundUpDiv(network.inputs, 4), roundUpDiv(network.nodesPerLayer, 4), roundUpDiv(network.networkCount, 4));

	/*glBindImageTexture(0, network.inputWeights, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUseProgram(ProgramMap[FillRandom2DArray]);
	glUniform1f(0, (float)trueRng() / trueRng.max());
	glDispatchCompute(network.inputs, network.nodesPerLayer, network.networkCount);*/
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//network weights
	for (int i = 0; i < network.networkWeights.size(); i++) {
		glBindImageTexture(0, network.networkWeights[i], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
		glUseProgram(ProgramMap[FillRandom4x4x4_2DArray]);
		glUniform1f(0, (float)trueRng() / trueRng.max());
		glUniform3i(1, network.nodesPerLayer, network.nodesPerLayer, network.networkCount);
		glDispatchCompute(roundUpDiv(network.nodesPerLayer, 4), roundUpDiv(network.nodesPerLayer, 4), roundUpDiv(network.networkCount, 4));
	}

	//network biases
	glBindImageTexture(0, network.networkBiases, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUseProgram(ProgramMap[FillRandom4x4x4_2DArray]);
	glUniform1f(0, (float)trueRng() / trueRng.max());
	glUniform3i(1, network.nodesPerLayer, network.layers, network.networkCount);
	glDispatchCompute(roundUpDiv(network.nodesPerLayer, 4), roundUpDiv(network.layers, 4), roundUpDiv(network.networkCount, 4));

	//output weights
	glBindImageTexture(0, network.outputWeights, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUseProgram(ProgramMap[FillRandom4x4x4_2DArray]);
	glUniform1f(0, (float)trueRng() / trueRng.max());
	glUniform3i(1, network.nodesPerLayer, network.outputs, network.networkCount);
	glDispatchCompute(roundUpDiv(network.nodesPerLayer, 4), roundUpDiv(network.outputs, 4), roundUpDiv(network.networkCount, 4));

	//output biases
	glBindImageTexture(0, network.outputBiases, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUseProgram(ProgramMap[FillRandom1DArray]);
	glUniform1f(0, (float)trueRng() / trueRng.max());
	glDispatchCompute(network.outputs, network.networkCount, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	////test
	float* testbuf = new float[network.inputs * network.nodesPerLayer * network.networkCount];
	glBindTexture(GL_TEXTURE_2D_ARRAY, network.inputWeights);
	glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RED, GL_FLOAT, testbuf);
	delete[] testbuf;

	return true;
}
bool evalNetworkGroup(glNeuralNetworkGroup& network, unsigned int inpbuf, unsigned int outbuf) {
	return false;
}
bool deleteNetworkGroup(glNeuralNetworkGroup& network) {
	glDeleteTextures(1, &(network.inputWeights));
	//glDeleteTextures(1, &(network.networkWeights));
	glDeleteTextures(1, &(network.networkBiases));
	glDeleteTextures(1, &(network.outputWeights));
	glDeleteTextures(1, &(network.outputBiases));
	return true;
}