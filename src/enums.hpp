#ifndef ENUMS_HPP
#define ENUMS_HPP
#include <cstdint>

enum class MaterialProperty : uint8_t
{
	None,
	Root,

	Name,

	AlphaCutoff,
	AlphaMode,
	DoubleSided,

	Normal,

	Occlusion,

	PBRMetallicRoughness,
	EmissiveFactor,
	EmissiveTexture,

	KHR_SpecularGlossiness,

	Unlit,

	NormalTexture,
	NormalScale,

	OcclusionTexture,
	OcclusionStrength,

	BaseColorFactor,
	BaseColorTexture,

	RoughnessFactor,
	MetallicFactor,
	MetallicRoughnessTexture,

	DiffuseFactor,
	DiffuseTexture,
	SpecularFactor,
	GlossinessFactor,
	SpecularGlossinessTexture,
	SpecularGlossinessEnabled,

	Total
};

enum class Heirarchy : uint8_t
{
	Root,
	ObjectList,
	ObjectContents,
	Material,
	Animation,
	Attachment,
	Textures,
	Total,
};

enum class Bitwise : uint8_t
{
	SET,
	AND,
	OR,
	XOR,
	NOT,
};

enum class TokType : uint8_t
{
	None,
	Boolean,
	AlphaMode,
	Float,
	Vec3,
	Vec4,
	String,
	Texture,
	Total
};

#endif // ENUMS_HPP
