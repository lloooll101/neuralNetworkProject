#include "glNetwork.hpp"
#include <glad/gl.h>
#include <random>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>



bool computeShadersCompiled = false;
enum GLComputePrograms {
	FillRandom1DArray, FillRandom2DArray,
	Set1D1, Set1D2,
	Set2D1, Set2D2
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
		if (!AddProgramToMap("resources/vecset1.comp", Set1D1)) break;
		if (!AddProgramToMap("resources/vecset2.comp", Set1D2)) break;
		if (!AddProgramToMap("resources/matset1.comp", Set2D1)) break;
		if (!AddProgramToMap("resources/matset2.comp", Set2D2)) break;

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

	unsigned int tex1;
	glGenTextures(1, &tex1);
	glBindTexture(GL_TEXTURE_2D, tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 512, 512, 0, GL_RED, GL_FLOAT, NULL);
	
	unsigned int tex2;
	glGenTextures(1, &tex2);
	glBindTexture(GL_TEXTURE_2D, tex2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 512, 512, 0, GL_RED, GL_FLOAT, NULL);

	glActiveTexture(GL_TEXTURE0);

	glBindImageTexture(0, tex1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUseProgram(ProgramMap[Set2D1]);
	glDispatchCompute(512, 512, 1);

	glBindImageTexture(0, tex2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUseProgram(ProgramMap[Set2D2]);
	glDispatchCompute(512, 512, 1);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	float* debugtex1 = new float[512*512];
	glBindTexture(GL_TEXTURE_2D, tex1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, debugtex1);

	float* debugtex2 = new float[512 * 512];
	glBindTexture(GL_TEXTURE_2D, tex2);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, debugtex2);

	delete[] debugtex1;
	delete[] debugtex2;
}

bool buildNetworkGroup(glNeuralNetworkGroup& network) {

	int maxTextureSize, maxTextureLayers;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTextureLayers);

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

	//create random values
	//create true rng for seed gen
	std::random_device trueRng;

	//input weights
	glBindImageTexture(0, network.inputWeights, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUseProgram(ProgramMap[FillRandom2DArray]);
	glUniform1f(0, (float)trueRng() / trueRng.max());
	glDispatchCompute(network.inputs, network.nodesPerLayer, network.networkCount);

	//network weights
	for (int i = 0; i < network.networkWeights.size(); i++) {
		glBindImageTexture(0, network.networkWeights[i], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
		glUseProgram(ProgramMap[FillRandom2DArray]);
		glUniform1f(0, (float)trueRng() / trueRng.max());
		glDispatchCompute(network.nodesPerLayer, network.nodesPerLayer, network.networkCount);
	}

	//network biases
	glBindImageTexture(0, network.networkBiases, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUseProgram(ProgramMap[FillRandom2DArray]);
	glUniform1f(0, (float)trueRng() / trueRng.max());
	glDispatchCompute(network.nodesPerLayer, network.layers, network.networkCount);

	//output weights
	glBindImageTexture(0, network.outputWeights, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUseProgram(ProgramMap[FillRandom2DArray]);
	glUniform1f(0, (float)trueRng() / trueRng.max());
	glDispatchCompute(network.nodesPerLayer, network.outputs, network.networkCount);

	//output biases
	glBindImageTexture(0, network.outputBiases, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUseProgram(ProgramMap[FillRandom1DArray]);
	glUniform1f(0, (float)trueRng() / trueRng.max());
	glDispatchCompute(network.outputs, network.networkCount, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	////test
	//float* testbuf = new float[network.inputs * network.nodesPerLayer * network.networkCount];
	//glBindTexture(GL_TEXTURE_2D_ARRAY, network.inputWeights);
	//glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RED, GL_FLOAT, testbuf);



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