#include "glNetwork.hpp"
#include <glad/gl.h>
#include <random>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>


bool computeShadersCompiled = false;
enum GLComputePrograms {
	FillRandom4x16x1_1DArray, FillRandom16x64x1_1DArray,
	FP16_FillRandom4x16x1_1DArray, FP16_FillRandom16x64x1_1DArray,

	FillRandom4x4x4_2DArray, FillRandom16x16x4_2DArray,
	FP16_FillRandom4x4x4_2DArray, FP16_FillRandom16x16x4_2DArray,

	FeedForwardW4D16
};
struct KernelSize {
	unsigned int x, y, z;
	KernelSize() = default;
	KernelSize(unsigned int x, unsigned int y, unsigned int z) : x{ x }, y{ y }, z{ z } {};
};
std::map<GLComputePrograms, unsigned int> ProgramMap;
std::map<GLComputePrograms, KernelSize> ProgramKernelSizes;

bool compileComputeShaders() {
	auto AddProgramToMap = [&](const std::string& filename, GLComputePrograms programtype, KernelSize kernelSize) -> bool {
		std::ifstream stream;
		std::stringstream ss;

		stream.open(filename);
		if (stream.fail()) {
			std::cout << "File " << filename << " could not be opened." << std::endl;
			return false;
		}
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
			std::cout << "In shader: " << filename << std::endl << log << std::endl;
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
			std::cout << "In shader: " << filename << std::endl << log << std::endl;
			delete[] log;
			return false;
		}
		try {
			ProgramMap.insert({ {programtype, program} });
			ProgramKernelSizes.insert({ {programtype, kernelSize} });
		}
		catch (std::exception) {
			return false;
		}
		
		return true;
	};

	bool success = false;

	do {
		if (!AddProgramToMap("resources/4x16x1rand1darr.comp", FillRandom4x16x1_1DArray, KernelSize(4, 16, 1))) break;
		if (!AddProgramToMap("resources/16x64x1rand1darr.comp", FillRandom16x64x1_1DArray, KernelSize(16, 64, 1))) break;
		if (!AddProgramToMap("resources/4x16x1fp16rand1darr.comp", FP16_FillRandom4x16x1_1DArray, KernelSize(4, 16, 1))) break;
		if (!AddProgramToMap("resources/16x64x1fp16rand1darr.comp", FP16_FillRandom16x64x1_1DArray, KernelSize(16, 64, 1))) break;

		if (!AddProgramToMap("resources/4x4x4rand2darr.comp", FillRandom4x4x4_2DArray, KernelSize(4, 4, 4))) break;
		if (!AddProgramToMap("resources/16x16x4rand2darr.comp", FillRandom16x16x4_2DArray, KernelSize(16, 16, 4))) break;
		if (!AddProgramToMap("resources/4x4x4fp16rand2darr.comp", FP16_FillRandom4x4x4_2DArray, KernelSize(4, 4, 4))) break;
		if (!AddProgramToMap("resources/16x16x4fp16rand2darr.comp", FP16_FillRandom16x16x4_2DArray, KernelSize(16, 16, 4))) break;

		if (!AddProgramToMap("resources/feedforward_w4d16.comp", FeedForwardW4D16, KernelSize(4, 1, 16))) break;

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

	int maxTextureSize, maxTextureLayers, maxThreadsPerInvocation;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTextureLayers);
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxThreadsPerInvocation);
	long maxWorkGroups[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (GLint*) & (maxWorkGroups[0]));
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (GLint*) &(maxWorkGroups[1]));
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (GLint*) &(maxWorkGroups[2]));

	if (network.layers <= 0 || network.nodesPerLayer <= 0 || network.inputs <= 0 || network.outputs <= 0 || network.networkCount <= 0) {
		std::cout << "Invalid network configuration!" << std::endl;
		return false;
	}
	
	//adjust networks to fit
	GLenum internalformat = GL_RGBA32F, format = GL_RGBA, type = GL_FLOAT;
	int adjustedNetworkCount = network.networkCount;
	adjustedNetworkCount = roundUpDiv(network.networkCount, 4);
	network.networkCount = adjustedNetworkCount * 4;

	if (network.flags.UseFP16) {
		internalformat = GL_RGBA16F;
		format = GL_RGBA;
		type = GL_HALF_FLOAT;
	}

	if (network.networkCount > maxTextureLayers) {
		std::cout << "Too many networks in one group! Consider reducing the amount of networks you are training or split up the networks required into multiple groups." << std::endl;
		return false;
	}
	if (network.inputs > maxTextureSize || network.nodesPerLayer > maxTextureSize || network.layers > maxTextureSize || network.outputs > maxTextureSize) {
		std::cout << "The OpenGL implementation on this device does not support networks of this size. Consider reducing your specified network size, such as layers, nodes per layer, inputs, and outputs." << std::endl;
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
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalformat, network.inputs, network.nodesPerLayer, adjustedNetworkCount, 0, format, type, nullptr);

	//network weights
	for (int i = 0; i < network.layers - 1; i++) {
		unsigned int texbuf;
		glGenTextures(1, &texbuf);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texbuf);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalformat, network.nodesPerLayer, network.nodesPerLayer, adjustedNetworkCount, 0, format, type, nullptr);
		network.networkWeights.push_back(texbuf);
	}

	//network biases
	glBindTexture(GL_TEXTURE_2D_ARRAY, network.networkBiases);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalformat, network.nodesPerLayer, adjustedNetworkCount, network.layers, 0, format, type, nullptr);

	//output weights
	glBindTexture(GL_TEXTURE_2D_ARRAY, network.outputWeights);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalformat, network.nodesPerLayer, network.outputs, adjustedNetworkCount, 0, format, type, nullptr);

	//output biases
	glBindTexture(GL_TEXTURE_1D_ARRAY, network.outputBiases);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, internalformat, network.outputs, adjustedNetworkCount, 0, format, type, nullptr);

	//build programs
	if (!computeShadersCompiled) {
		if (!compileComputeShaders()) {
			return false;
		}
	}

	//get appropriate programs and kernel sizes
	GLComputePrograms prog2d, prog3d;
	KernelSize prog2d_ksize, prog3d_ksize;
	if (network.flags.UseLargeKernels) {
		prog2d = FillRandom16x64x1_1DArray;
		prog3d = FillRandom16x16x4_2DArray;
		if (network.flags.UseFP16) {
			prog2d = FP16_FillRandom16x64x1_1DArray;
			prog3d = FP16_FillRandom16x16x4_2DArray;
		}
	}
	else {
		prog2d = FillRandom4x16x1_1DArray;
		prog3d = FillRandom4x4x4_2DArray;
		if (network.flags.UseFP16) {
			prog2d = FP16_FillRandom4x16x1_1DArray;
			prog3d = FP16_FillRandom4x4x4_2DArray;
		}
	}
	prog2d_ksize = ProgramKernelSizes[prog2d];
	prog3d_ksize = ProgramKernelSizes[prog3d];

	//create random values
	//create true rng for seed gen
	std::random_device trueRng;

	//input weights
	glBindImageTexture(0, network.inputWeights, 0, GL_TRUE, 0, GL_WRITE_ONLY, internalformat);
	glUseProgram(ProgramMap[prog3d]);
	glUniform4f(0, (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max());
	glUniform3i(1, network.inputs, network.nodesPerLayer, adjustedNetworkCount);
	glDispatchCompute(roundUpDiv(network.inputs, prog3d_ksize.x), roundUpDiv(network.nodesPerLayer, prog3d_ksize.y), roundUpDiv(adjustedNetworkCount, prog3d_ksize.z));

	//network weights
	for (int i = 0; i < network.networkWeights.size(); i++) {
		glBindImageTexture(0, network.networkWeights[i], 0, GL_TRUE, 0, GL_WRITE_ONLY, internalformat);
		glUseProgram(ProgramMap[prog3d]);
		glUniform4f(0, (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max());
		glUniform3i(1, network.nodesPerLayer, network.nodesPerLayer, adjustedNetworkCount);
		glDispatchCompute(roundUpDiv(network.nodesPerLayer, prog3d_ksize.x), roundUpDiv(network.nodesPerLayer, prog3d_ksize.y), roundUpDiv(adjustedNetworkCount, prog3d_ksize.z));
	}

	//network biases
	glBindImageTexture(0, network.networkBiases, 0, GL_TRUE, 0, GL_WRITE_ONLY, internalformat);
	glUseProgram(ProgramMap[prog3d]);
	glUniform4f(0, (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max());
	glUniform3i(1, network.nodesPerLayer, network.layers, adjustedNetworkCount);
	glDispatchCompute(roundUpDiv(network.nodesPerLayer, prog3d_ksize.x), roundUpDiv(adjustedNetworkCount, prog3d_ksize.y), roundUpDiv(network.layers, prog3d_ksize.z));

	//output weights
	glBindImageTexture(0, network.outputWeights, 0, GL_TRUE, 0, GL_WRITE_ONLY, internalformat);
	glUseProgram(ProgramMap[prog3d]);
	glUniform4f(0, (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max());
	glUniform3i(1, network.nodesPerLayer, network.outputs, adjustedNetworkCount);
	glDispatchCompute(roundUpDiv(network.nodesPerLayer, prog3d_ksize.x), roundUpDiv(network.outputs, prog3d_ksize.y), roundUpDiv(adjustedNetworkCount, prog3d_ksize.z));

	//output biases
	glBindImageTexture(0, network.outputBiases, 0, GL_TRUE, 0, GL_WRITE_ONLY, internalformat);
	glUseProgram(ProgramMap[prog3d]);
	glUniform4f(0, (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max(), (float)trueRng() / trueRng.max());
	glDispatchCompute(roundUpDiv(network.outputs, prog2d_ksize.x), roundUpDiv(adjustedNetworkCount, prog2d_ksize.y), 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//test
	float* testbuf = new float[network.inputs * network.nodesPerLayer * network.networkCount];
	glBindTexture(GL_TEXTURE_2D_ARRAY, network.inputWeights);
	glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, format, type, testbuf);
	delete[] testbuf;

	return true;
}

//inpbuf should be 1d_array texture with inputs * (networks/4) dimensions with the texturees holding 4 floats per pixel (gl_rgba32f)
//outbuf is the same thing but with outpus instead of inputs
bool evalNetworkGroup(glNeuralNetworkGroup& network, unsigned int inpbuf, unsigned int outbuf) {
	//how to solve a neural network tutorial
	//
	// 1. build double temp buffers
	//    temp buffer should have the width of max(outputs, nodesPerLayer) 
	// 
	// Note: step forward function = sigmoid((weights*inpvector) + biases)
	// 
	// 2. repeatedly solve step forward function
	//

	//create test inputs
	unsigned int dbginp;
	glGenTextures(1, &dbginp);
	glBindTexture(GL_TEXTURE_1D_ARRAY, dbginp);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	std::random_device rd;
	float* randomNumbers = new float[network.inputs * network.networkCount];
	for (int i = 0; i < network.inputs * network.networkCount; i++) {
		randomNumbers[i] = (float)rd() / rd.max();
	}

	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_RGBA32F, network.inputs, network.networkCount / 4, 0, GL_RGBA, GL_FLOAT, randomNumbers);
	//delete[] randomNumbers;


	//compile shaders
	if (!computeShadersCompiled) {
		if (!compileComputeShaders()) {
			return false;
		}
	}

	//get programs
	GLComputePrograms ffprogram;
	KernelSize ffprogram_ksize;
	ffprogram = FeedForwardW4D16;
	ffprogram_ksize = ProgramKernelSizes[ffprogram];

	//building buffers
	GLenum internalformat = GL_RGBA32F, format = GL_RGBA, type = GL_FLOAT;
	if (network.flags.UseFP16) {
		internalformat = GL_RGBA16F;
		format = GL_RGBA;
		type = GL_HALF_FLOAT;
	}

	unsigned int tempBuffers[2];
	glGenTextures(2, tempBuffers);

	glBindTexture(GL_TEXTURE_1D_ARRAY, tempBuffers[0]);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, internalformat, std::max(network.nodesPerLayer, network.outputs), network.networkCount / 4, 0, format, type, nullptr);

	glBindTexture(GL_TEXTURE_1D_ARRAY, tempBuffers[1]);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, internalformat, std::max(network.nodesPerLayer, network.outputs), network.networkCount / 4, 0, format, type, nullptr);

	//create swapchain (vulkan reference????)
	int currentWriteBuffer = 0;
	int currentReadBuffer = 0;

	//start stepping forward
	//start with input layer
	glBindImageTexture(0, network.inputWeights, 0, GL_TRUE, 0, GL_READ_ONLY, internalformat);
	glBindImageTexture(1, /*inpbuf*/ dbginp, 0, GL_TRUE, 0, GL_READ_ONLY, internalformat);
	glBindImageTexture(2, network.networkBiases, 0, GL_FALSE, 0, GL_READ_ONLY, internalformat);
	glBindImageTexture(3, tempBuffers[currentWriteBuffer], 0, GL_TRUE, 0, GL_WRITE_ONLY, internalformat);
	glUseProgram(ProgramMap[FeedForwardW4D16]);
	glUniform3i(0, network.inputs, network.nodesPerLayer, network.networkCount/4);
	glDispatchCompute(roundUpDiv(network.nodesPerLayer, ffprogram_ksize.x), 1, roundUpDiv(network.networkCount, ffprogram_ksize.z));
	
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//test
	float* testbuf = new float[std::max(network.nodesPerLayer, network.outputs) * network.networkCount];
	glBindTexture(GL_TEXTURE_1D_ARRAY, tempBuffers[currentWriteBuffer]);
	glGetTexImage(GL_TEXTURE_1D_ARRAY, 0, format, type, testbuf);
	delete[] testbuf;

	return false;
}
bool deleteNetworkGroup(glNeuralNetworkGroup& network) {
	//glDeleteTextures(1, &(network.inputWeights));
	//glDeleteTextures(1, &(network.networkWeights));
	//glDeleteTextures(1, &(network.networkBiases));
	//glDeleteTextures(1, &(network.outputWeights));
	//glDeleteTextures(1, &(network.outputBiases));
	return false;
}