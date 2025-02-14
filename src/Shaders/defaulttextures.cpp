#include "defaulttextures.h"
#include "widgets/glviewwidget.h"
#include <atomic>
#include <stdexcept>
#include <cstring>

DefaultTextures::DefaultTextures() :
	refCount(0)
{
	memset(textures, 0, sizeof(textures));
}

DefaultTextures::~DefaultTextures()
{
}

void DefaultTextures::createTextures(GLViewWidget * gl)
{
	if(refCount.load() == 0)
		throw std::runtime_error("Tried to create default textures without incing refcount...");

	_gl glGenTextures(TotalTextures, textures);

	uint8_t pixels[TotalTextures][4] =
	{
		{0xFF, 0xFF, 0xFF, 0xFF},
		{0x80, 0x80, 0xFF, 0xFF},
		{0x01, 0x01, 0x01, 0x80},
		{0xFF, 0x80, 0x00, 0xFF}
	};


	for(int i = 0; i < TotalTextures; ++i)
	{
		_gl glBindTexture(GL_TEXTURE_2D, textures[i]);

		_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

		_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

		_gl glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		_gl glTexImage2D(
			GL_TEXTURE_2D,  //target
			0,	//level
			GL_RGBA, //internal format
			1, //width
			1, //height,
			0, //border must be 0
			GL_RGBA, //format of incoming source
			GL_UNSIGNED_BYTE, //specific format
			&pixels[i][0]);
	}
}

void DefaultTextures::destroyTextures(GLViewWidget * gl)
{
	_gl glDeleteTextures(TotalTextures, textures);
	memset(textures, 0, sizeof(textures));
}
