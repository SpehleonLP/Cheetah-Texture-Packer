#include "widgets/glviewwidget.h"

#include "Support/glm_iostream.hpp"

#include "imagesupport.h"
#include "Import/import_c16.h"

#include <iostream>
#include <glm/glm.hpp>
#include <cctype>
#include <cstring>
#include "lf_math.h"

#if USE_BASISU

#else
#include "Support/qt_to_gl.h"
#include <QImageReader>
#include <QImageWriter>
#include <QImage>
#endif


#undef LoadImage

void IO::UploadImage(GLViewWidget * gl, uint32_t * texture, uint8_t * data, glm::i16vec2 size, uint32_t internal_format, uint32_t format, uint32_t type)
{
	GL_ASSERT;

	(void)gl;

	if(texture == nullptr)
		return;

	if(*texture == 0)
		_gl glGenTextures(1, texture);

	_gl glBindTexture(GL_TEXTURE_2D, *texture);

	_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	_gl glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	_gl glTexImage2D(GL_TEXTURE_2D,
			0,
			internal_format,
			size.x,
			size.y,
			0,
			format,
			type,
			&data[0]);

	GL_ASSERT;
}

CountedSizedArray<glm::i16vec4> IO::GetSprites(uint8_t * data, uint32_t, glm::i16vec2 size, int channels)
{
	std::vector<glm::i16vec4> r;

	uint32_t color;
	memcpy(&color, &data[0], channels);

	auto ComparePx = [=](int x, int y)
	{
		return !memcmp(&color, &data[(y * size.x + x)*channels], channels);
	};

	for(int y = 0; y < size.y; ++y)
		for(int x = 0; x < size.x; ++x)
		{
			if(ComparePx(x, y))
				continue;

//top left corner i guess?
			if(y == 0 || ComparePx(x, y-1))
			{
				r.push_back(GetSprite(data, size, channels, glm::i16vec2(x, y), color));
				x = r.back().z;
				continue;
			}

			for(auto j = r.rbegin(); j != r.rend(); ++j)
			{
				if(math::contains(*j, glm::i16vec2(x, y)))
				{
					x = j->z;
					goto end;
				}
				else if(j->w < y)
					break;
			}

//			throw std::logic_error("missed a sprite?");
end:
			(void)0;
		}

	std::cerr << "total sprites found: " << r.size() << std::endl;

	return CountedSizedArray<glm::i16vec4>::FromArray(&r[0], r.size());
}

CountedSizedArray<glm::i16vec4> IO::GetCrop(uint8_t * data, uint32_t data_bytes, glm::i16vec2 size, int channels, ConstSizedArray<glm::i16vec4> sprites)
{
	assert(data_bytes >= (uint32_t) (size.x * size.y * channels));

	CountedSizedArray<glm::i16vec4> r(sprites.size());

	uint32_t mask = channels == 4? 0x000000FF : 0xFFFFFF00;

	for(uint32_t i = 0; i < sprites.size(); ++i)
	{
		r[i] = GetCrop(data, size, channels, sprites[i], mask, 0);
	}

	return r;
}

CountedSizedArray<glm::u16vec4> IO::NormalizeCrop(ConstSizedArray<glm::i16vec4> sprites, glm::i16vec2 size)
{
	CountedSizedArray<glm::u16vec4> r(sprites.size());

	size = glm::max(size, glm::i16vec2(1, 1));

	for(uint32_t i = 0; i < sprites.size(); ++i)
	{
		glm::dvec4 uv(
			sprites[i].x / (double) size.x,
			sprites[i].y / (double) size.y,
			sprites[i].z / (double) size.x,
			sprites[i].w / (double) size.y);

		uv = glm::clamp(uv, glm::dvec4(0), glm::dvec4(1));
		r[i] = uv * (double) USHRT_MAX;
	}

	return r;
}


glm::i16vec4 IO::GetSprite(uint8_t * data, glm::i16vec2 size, int channels, glm::i16vec2 tl, uint32_t color)
{
	glm::i16vec2 i  = tl;
	glm::i16vec2 br = size;

	auto ComparePx = [=](int x, int y)
	{
		return !memcmp(&color, &data[(y * size.x + x)*channels], channels);
	};

//scan top
	for(; i.x < size.x; ++i.x)
	{
		if(i.y > 0 && !ComparePx(i.x, i.y-1))
			throw std::runtime_error("sprite beginning at " + std::to_string(tl) + " does not have a straight upper edge." );

		if(ComparePx(i.x, i.y))
		{
			br.x = i.x;
			break;
		}
	}

//scan left
	for(i = tl; i.y < size.y; ++i.y)
	{
		if(i.x > 0 && !ComparePx(i.x-1, i.y))
			throw std::runtime_error("sprite beginning at " + std::to_string(tl) + " does not have a straight left edge." );

		if(ComparePx(i.x, i.y))
		{
			br.y = i.y;
			break;
		}
	}

	if(br.x < size.x && br.y+1 < size.y
	&& !ComparePx(br.x, br.y))
		throw std::runtime_error("sprite beginning at " + std::to_string(tl) + " is not proper square." );


//verify bottom
	for(i = glm::ivec2(tl.x, br.y); i.x < br.x; ++i.x)
	{
		if((i.y+1 < size.y && !ComparePx(i.x, i.y))
		|| ComparePx(i.x, i.y-1))
			throw std::runtime_error("sprite beginning at " + std::to_string(tl) + " does not have a straight bottom edge." );
	}

//verify right
	for(i = glm::ivec2(br.x, tl.y); i.y < br.y; ++i.y)
	{
		if((i.x+1 < size.x && !ComparePx(i.x, i.y))
		|| ComparePx(i.x-1, i.y))
			throw std::runtime_error("sprite beginning at " + std::to_string(tl) + " does not have a straight right edge." );
	}

	std::cerr << "found sprite: " << glm::i16vec4(tl.x, tl.y, br.x, br.y) << std::endl;

	return glm::i16vec4(tl.x, tl.y, br.x, br.y);
}

glm::i16vec4 IO::GetCrop  (uint8_t const* data, glm::i16vec2 size, int channels, glm::i16vec4 aabb, uint32_t greenMask, uint32_t greenScreen)
{
	glm::i16vec2 min{aabb.z - aabb.x, aabb.w - aabb.y};
	glm::i16vec2 max{0, 0};
	glm::i16vec2 N = min;

	uint32_t px   = 0x000000FF;

	for(short y = 0; y < N.y; ++y)
	{
		auto row = &data[((y+aabb.y) *size.x + aabb.x)*channels];

		for(short x = 0; x < N.x; ++x)
		{
			memcpy(&px, &row[x*channels], channels);

			if((px & greenMask) != greenScreen)
			{
				min = glm::min(min, glm::i16vec2(x, y));
				max = glm::max(max, glm::i16vec2(x, y));
			}
		}
	}

	return glm::i16vec4(glm::min(min, max) + glm::i16vec2(aabb.x, aabb.y), max + glm::i16vec2(aabb.x, aabb.y));
}


void IO::DownloadImage(GLViewWidget * gl, IO::Image * image,  uint32_t texture, int internalFormat, int format, int type)
{
	assert(image != nullptr);

	GL_ASSERT;
	(void)gl;

	if(texture == 0)
		return;

	int width{}, height{};
	_gl glBindTexture(GL_TEXTURE_2D, texture);	GL_ASSERT;
	_gl glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH , &width);	GL_ASSERT;
	_gl glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);	GL_ASSERT;

	if(internalFormat < 0)
	{
		_gl glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT , (int*)&internalFormat);	GL_ASSERT;
	}

	image->size           = glm::ivec2(width, height);
	image->internalFormat = internalFormat;
	image->format         = format < 0?  Qt_to_Gl::GetFormatFromInternalFormat(internalFormat) : format;
	image->type           = type < 0? Qt_to_Gl::GetTypeFromInternalFormat(internalFormat) : type;

	int bytes_per_pixel = Qt_to_Gl::GetPixelByteWidth(image->format, image->type);
	image->bytes          = image->size.x * image->size.y * bytes_per_pixel;

	image->image.reset(new uint8_t[image->bytes]);

	_gl glGetTexImage(GL_TEXTURE_2D, 0, image->format, image->type, &image->image[0]);
	GL_ASSERT;
}


bool IO::CheckDynamics(std::string & error, float & size_ratio, ConstSizedArray<glm::i16vec4> A, ConstSizedArray<glm::i16vec4> B)
{
	if(A == B) return true;

	if(A.size() != B.size())
	{
		error = "number of sprites in sprite sheet does not match previous sheets";
		return false;
	}

	float sr = size_ratio;

	for(float i = 0; i < A.size(); ++i)
	{
		glm::i16vec2 size0(A[i].z - A[i].x,	A[i].w - A[i].y);
		glm::i16vec2 size1(B[i].z - B[i].x, B[i].w - B[i].y);

		if(std::fabs(size0.x / (float) size0.y - size1.x / (float) size1.y) > .0001)
		{
			error = "aspect ratio of cropped sprite " + std::to_string(i) + " is not the same as in previous images";
			return false;
		}

		if(sr < 0)
			sr = size0.x / (float) size1.x;
		else if(std::fabs(sr - size0.x / (float) size1.x) > .0001)
		{
			error =  "all sprites in sprite sheet must have same % of size compared to original sheet.";
			return false;
		}
	}

	size_ratio = sr;

	return true;
}


#if !USE_BASISU
#include "Support/qt_to_gl.h"
#include <QImageReader>
#include <QImageWriter>
#include <QByteArray>
#include <QBuffer>
#include <QImage>
#include <QString>
#include <QDir>

void IO::DownloadImage(GLViewWidget * gl, QImage * image,  uint32_t texture)
{
	GL_ASSERT;
	(void)gl;

	if(texture == 0)
		return;

	int width, height;
	_gl glBindTexture(GL_TEXTURE_2D, texture);
	_gl glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH , &width);
	_gl glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	uint8_t * ptr = (uint8_t*) malloc(width*height*4);
	memset(ptr, 0, width*height*4);
	_gl glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, ptr);

	*image = QImage(ptr, width, height, 4*width, QImage::Format_ARGB32, &std::free, ptr);

	GL_ASSERT;
}


void IO::ImageToPngData(GLViewWidget * gl, uint32_t id, std::unique_ptr<uint8_t[]> * array, uint32_t * size)
{
	QImage image;

	DownloadImage(gl, &image, id);

	ImageToPngData(&image, array, size);
}

void IO::ImageToPngData(QImage * image, std::unique_ptr<uint8_t[]> * array, uint32_t * size)
{
	if(image->isNull())
	{
		*size = 0;
		return;
	}

	QBuffer device;
	QImageWriter writer(&device, "png");
	writer.write(*image);

	auto & buffer = device.buffer();

	*size = buffer.length();
	if(size == 0)
		return;

	array->reset(new uint8_t[*size]);
	memcpy(&(*array)[0], buffer.constData(), *size);
}

std::string IO::MimeType(const char * path)
{
	QImageReader reader(path);
	QString result = "image/" + reader.format();
	return result.toStdString();
}

IO::Image IO::LoadImage(const char * path)
{
	QImageReader reader(path);
	reader.setAutoTransform(true);
	QImage newImage = reader.read();

	if (newImage.isNull()) {
		throw std::runtime_error(
			QString("Cannot load %1: %2").arg(
				QDir::toNativeSeparators(path),
				reader.errorString()).toStdString());
    }

	IO::Image image;
//convert image to proper format

	QImage::Format GetTargetFormat(QImage::Format in, QImage::Format);

	bool uses_alpha = Qt_to_Gl::ImageUsesAlpha(newImage);

	auto format = uses_alpha? QImage::Format_ARGB32 : QImage::Format_RGB888; //Qt_to_Gl::GetTargetFormat(newImage);

	if(format != newImage.format())
		newImage = newImage.convertToFormat(format,
			Qt::AutoColor
			| Qt::DiffuseDither
			| Qt::DiffuseAlphaDither
			| Qt::PreferDither);

	image.size = glm::i16vec2(newImage.width(), newImage.height());
	image.format         = Qt_to_Gl::GetFormat(format);
	image.internalFormat = Qt_to_Gl::GetInternalFormat(format);
	image.type           = Qt_to_Gl::GetType(format);
	image.bytes          =  image.size.y*newImage.bytesPerLine();

	image.image.reset(new uint8_t[image.bytes]);

	memcpy(&image.image[0], newImage.constBits(), image.bytes);

	return image;
}

#else //USE_BASISU

const char * g_ReadMimeTypes[] =
{
"image/jpg",
"image/png",
"image/bmp"
""
};

const char * g_WriteMimeTypes[] =
{
"image/png",
"image/bmp"
""
};

IO::Format IO::GetFormat(const char * path, int length)
{
	struct FormatTable
	{
		const char * extension;
		uint8_t        length;
		IO::Format format;
	};

	static FormatTable table[] =
	{
	{ ".bmp",   4, IO::Format::BMP },
	{ ".jpg",   4, IO::Format::JPEG },
	{ ".jpeg",  5, IO::Format::JPEG },
	{ ".png",   4, IO::Format::PNG },
	//{ ".tga", 4, IO::Format::TGA },
	{ nullptr,  0, IO::Format::None }
	};

	if(path == nullptr)
		return Format::None;

	for(FormatTable * fmt = table; fmt->extension; ++fmt)
	{
//		std::cerr << "checking format: " << fmt->extension << std::endl;

		auto ext = fmt->extension + fmt->length;
		auto pth = path + (length >= 0? length : strlen(path));

		for(;ext >= fmt->extension && pth >= path; --pth, --ext)
		{
			if(*ext != tolower(*pth))
				break;

			if(*ext == '.')
				return fmt->format;
		}
	}

	return Format::Unknown;
}

IO::Image IO::LoadImage(const char * path)
{
	IO::Image image;

	switch(GetFormat(path))
	{
	case IO::Format::Unknown:
		throw std::runtime_error(std::string(path) + "\n Unkown Image Format");
	case IO::Format::None:
		return image;
	case IO::Format::BMP:
	{
		uint32_t compz{};
		int width{}, height{};
		auto data = apg_bmp_read(path, &width, &height, &compz);

		image.image.reset(data);
		image.size = glm::ivec2(width, height);
		image.channels = compz;
		return image;
	}
	case IO::Format::JPEG:
	{
		int compz, width, height;
		auto data = jpgd::decompress_jpeg_image_from_file(path, &width, &height, &compz, 4, 0);

		image.image.reset(data);
		image.size = glm::ivec2(width, height);
		image.channels = compz;
		break;
	}
	case IO::Format::PNG:
	{
		uint32_t x, y;
		uint8_t * data{nullptr};
		auto r = lodepng_decode32_file(&data, &x, &y, path);

		if(r != 0)
			throw std::runtime_error("problem reading png");

		image.image.reset(data);
		image.size = glm::ivec2(x, y);
		image.channels = 4;
		break;
	} break;
	default:
		throw std::logic_error("unhandled image format");
	}

	return image;
}

void IO::SaveImage(const char * path, uint8_t * data, glm::i16vec2 size, int channels)
{
	switch(GetFormat(path))
	{
	case IO::Format::Unknown:
		throw std::runtime_error("Problem saving: " + std::string(path) + "\n Unknown image format.");
	case IO::Format::None:
		return;
	case IO::Format::BMP:
		if(!apg_bmp_write(path, data, size.x, size.y, channels))
			throw std::runtime_error("Problem saving bitmap: " + std::string(path));
		break;
	case IO::Format::JPEG:
	{
		throw std::runtime_error("Problem saving: " + std::string(path) + "\n Saving jpegs is unsupported at this time.");
		break;
	}
	case IO::Format::PNG:
	{
		LodePNGColorType types[] = { LCT_GREY, LCT_GREY_ALPHA, LCT_RGB, LCT_RGBA };
		if(lodepng_encode_file(path, data, size.x, size.y, types[channels % 4], 8))
			throw std::runtime_error("Problem saving png: " + std::string(path));
	} break;
	default:
		throw std::logic_error("unhandled image format");
	}
}

void IO::DownloadImage(GLViewWidget * gl, basisu::image & image,  uint32_t texture)
{
	GL_ASSERT;
	(void)gl;

	if(texture == 0)
		return;

	int width, height;
	_gl glBindTexture(GL_TEXTURE_2D, texture);
	_gl glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH , &width);
	_gl glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	image.resize(width, height);

	_gl glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, image.get_ptr());
	GL_ASSERT;
}

#endif
