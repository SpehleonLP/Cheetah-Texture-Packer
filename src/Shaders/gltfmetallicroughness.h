#ifndef GLTFMETALLICROUGHNESS_H
#define GLTFMETALLICROUGHNESS_H
#include "spriteshaderbase.h"
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <atomic>

struct Material;

class gltfMetallicRoughness : public SpriteShaderBase
{
public:
	static gltfMetallicRoughness Shader;

	void bind(GLViewWidget* gl, Material * material);

private:
    void construct(GLViewWidget* gl);
	void destruct(GLViewWidget* gl);

	int32_t u_normal;
	int32_t u_occlusion;
	int32_t u_diffuse;
	int32_t u_pbr;

	int32_t u_alphaMode;
	int32_t u_texCoords;
	int32_t u_NOMR;
	int32_t u_specularFactor;
	int32_t u_baseColorFactor;
	int32_t u_emissionFactor;
};

#endif // GLTFMETALLICROUGHNESS_H
