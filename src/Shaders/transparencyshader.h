#ifndef TRANSPARENCYSHADER_H
#define TRANSPARENCYSHADER_H
#include "spriteshaderbase.h"
#include "glprogram.h"
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <atomic>

class TransparencyShader : public SpriteShaderBase
{
public:
static TransparencyShader Shader;
    virtual ~TransparencyShader() = default;

    void bind(GLViewWidget* gl, Material *) override;

private:
    void construct(GLViewWidget* gl) override;
	void destruct(GLViewWidget* gl) override;

	int32_t u_spriteSize;
};

#endif // TRANSPARENCYSHADER_H
