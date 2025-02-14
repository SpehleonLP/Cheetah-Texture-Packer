#ifndef VELVETSHADER_H
#define VELVETSHADER_H
#include "glprogram.h"
#include <glm/vec4.hpp>
#include <atomic>

class VelvetShader : public glProgram
{
public:
static VelvetShader Shader;
    void bind(GLViewWidget* gl, glm::vec4 color);

private:
	void construct(GLViewWidget* gl);
	void destruct(GLViewWidget* gl);

	std::atomic<int> refCount{0};
	uint32_t         texture{0};

	int32_t          u_texture;
	int32_t          u_color;
	int32_t          u_texSize;
};

#endif // VELVETSHADER_H
