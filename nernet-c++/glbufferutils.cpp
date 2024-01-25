#include <glad/gl.h>
#include <random>
#include <map>

//get these from glnetwork
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
extern std::map<GLComputePrograms, unsigned int> ProgramMap;
extern std::map<GLComputePrograms, KernelSize> ProgramKernelSizes;

extern bool computeShadersCompiled;
bool compileComputeShaders();
long roundUpDiv(long dividend, long divisor);

unsigned int Create1DArrayBuffer(int width, int layers, bool fp16 = false) {
    unsigned int buf;
    glGenTextures(1, &buf);
    glBindTexture(GL_TEXTURE_1D_ARRAY, buf);
    glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, fp16 ? GL_RGBA16F : GL_RGBA32F, width, roundUpDiv(layers, 4), 0, GL_RGBA, fp16 ? GL_HALF_FLOAT : GL_FLOAT, nullptr);

    return buf;
}

void DeleteArrayBuffer(unsigned int buffer) {
    glDeleteTextures(1, &buffer);
}

void Fill1DArrayRandom(unsigned int buffer, int width, int layers, unsigned long long seed, bool fp16 = false) {
    GLComputePrograms rngkernel = fp16 ? FP16_FillRandom4x16x1_1DArray : FillRandom4x16x1_1DArray;
    KernelSize kernelSize = ProgramKernelSizes[rngkernel];

    glBindImageTexture(0, )
}

void Fill1DArrayRandom(unsigned int buffer, int width, int layers, bool fp16 = false) {

}