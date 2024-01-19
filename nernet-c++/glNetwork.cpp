#include "glNetwork.hpp"
#include <glad/gl.h>
#include <random>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

void test() {
	std::ifstream f;
	std::stringstream ss;
	f.open("vecadd1.comp");
	if(f.fail()) return;
	ss << f.rdbuf();
	f.close();
	std::string compstr = ss.str();
	const char* ccstr = compstr.c_str();

	unsigned int cshader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(cshader, 1, &ccstr, NULL);
	glCompileShader(cshader);
	
	unsigned int program = glCreateProgram();
	glAttachShader(program, cshader);
	glLinkProgram(program);
	glDeleteShader(cshader);

	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_1D, tex);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R8, 200, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

	glActiveTexture(GL_TEXTURE0);

	glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
	glUseProgram(program);
	glDispatchCompute(200, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	unsigned char* debugbuffer = new unsigned char[200];
	glBindTexture(GL_TEXTURE_1D, tex);
	glGetTexImage(GL_TEXTURE_1D, 0, GL_RED, GL_UNSIGNED_BYTE, debugbuffer);
	int a;
	a = 1;
	a = a + a;
}

bool computeShadersCompiled = false;
enum GLComputePrograms {
	FillRandom1D, FillRandom1DArray,
	FillRandom2D, FillRandom2DArray
};
std::map<GLComputePrograms, unsigned int> ProgramMap;

unsigned int fillRandom1D_Program;
unsigned int fillRandom1DArr_Program;
unsigned int fillRandom2D_Program;
unsigned int fillRandom2DArr_Program;

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
			glGetShaderInfoLog(shader, 0, &log_length, nullptr);
			char* log = new char[log_length + 1];
			log[log_length] = '\0';
			glGetShaderInfoLog(shader, log_length, nullptr, log);
			std::cout << log << std::endl;
			delete log;
			return false;
		}

		unsigned int program = glCreateProgram();
		glAttachShader(program, shader);
		glLinkProgram(program);
		glDeleteShader(shader);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			int log_length;
			glGetProgramInfoLog(program, 0, &log_length, nullptr);
			char* log = new char[log_length + 1];
			log[log_length] = '\0';
			glGetProgramInfoLog(program, log_length, nullptr, log);
			std::cout << log << std::endl;
			delete log;
			return false;
		}
		try {
			ProgramMap.insert({ {programtype, program} });
		}
		catch (std::exception) {
			return false;
		}
		
	};

	bool success = false;

	do {
		if (!AddProgramToMap("fillrandom1d.comp", FillRandom1D)) break;
		if (!AddProgramToMap("fillrandom1darr.comp", FillRandom1DArray)) break;
		if (!AddProgramToMap("fillrandom2d.comp", FillRandom2D)) break;
		if (!AddProgramToMap("fillrandom2darr.comp", FillRandom2DArray)) break;

		success = true;
	} while (false);
	
	if (!success) {
		return false;
	}
	computeShadersCompiled = true;
	return true;
}



bool buildNetworkGroup(glNeuralNetworkGroup& network) {

	int a, b;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &a);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &b);


	if (network.layers <= 0 || network.nodesPerLayer <= 0 || network.inputs <= 0 || network.outputs <= 0 || network.networkCount <= 0) {
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
	glUseProgram(fillRandom2DArr_Program);
	glDispatchCompute(network.inputs, network.nodesPerLayer, network.networkCount);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	random = (float)rd() / rd.max();
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 1, GL_RED, GL_FLOAT, (const void*)&random);


	//network weights
	glBindImageTexture(0, network.networkWeights, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindImageTexture(1, rngseedtex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glUseProgram(fillRandom2DArr_Program);
	glDispatchCompute(network.nodesPerLayer, network.nodesPerLayer, (network.layers - 1)*network.networkCount);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	random = (float)rd() / rd.max();
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 1, GL_RED, GL_FLOAT, (const void*)&random);

	//network biases
	glBindImageTexture(0, network.networkBiases, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindImageTexture(1, rngseedtex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glUseProgram(fillRandom2DArr_Program);
	glDispatchCompute(network.nodesPerLayer, network.layers, network.networkCount);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	random = (float)rd() / rd.max();
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 1, GL_RED, GL_FLOAT, (const void*)&random);

	//output weights
	glBindImageTexture(0, network.outputWeights, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindImageTexture(1, rngseedtex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glUseProgram(fillRandom2DArr_Program);
	glDispatchCompute(network.nodesPerLayer, network.outputs, network.networkCount);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	random = (float)rd() / rd.max();
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 1, GL_RED, GL_FLOAT, (const void*)&random);

	//output biases
	glBindImageTexture(0, network.outputBiases, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindImageTexture(1, rngseedtex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glUseProgram(fillRandom1DArr_Program);
	glDispatchCompute(network.outputs, fillRandom2DArr_Program, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glDeleteTextures(1, &rngseedtex);
	return true;
}
bool evalNetworkGroup(glNeuralNetworkGroup& network, unsigned int outbuf) {
	return false;
}
bool deleteNetworkGroup(glNeuralNetworkGroup& network) {
	glDeleteTextures(1, &(network.inputWeights));
	glDeleteTextures(1, &(network.networkWeights));
	glDeleteTextures(1, &(network.networkBiases));
	glDeleteTextures(1, &(network.outputWeights));
	glDeleteTextures(1, &(network.outputBiases));
	return true;
}