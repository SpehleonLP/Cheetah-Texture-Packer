#ifndef DEFAULTTEXTURES_H
#define DEFAULTTEXTURES_H
#include <atomic>

class GLViewWidget;

class DefaultTextures
{
public:
	static DefaultTextures & Get()
	{
		static DefaultTextures defaults;
		return defaults;
	}

	~DefaultTextures();

	inline void AddRef() { ++refCount; }
	inline void Release(GLViewWidget * gl) { if(--refCount == 0) destroyTextures(gl); }

#define TextureGetter(x) \
	uint32_t Get##x(GLViewWidget * gl) { \
		if(textures[x] == 0)	createTextures(gl); \
		return textures[x]; \
	}

	TextureGetter(WhiteTexture)
	TextureGetter(NormalTexture)
	TextureGetter(SpecularGlossinessTexture)
	TextureGetter(MetallicRoughnessTexture)

private:
	enum
	{
		WhiteTexture,
		NormalTexture,
		SpecularGlossinessTexture,
		MetallicRoughnessTexture,
		TotalTextures
	};

	std::atomic<int> refCount;
	uint32_t textures[TotalTextures];

	DefaultTextures();

	void createTextures(GLViewWidget * gl);
	void destroyTextures(GLViewWidget * gl);
};

#endif // DEFAULTTEXTURES_H
