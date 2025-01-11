#include "qt_to_gl.h"
#include <stdexcept>
#include <QOpenGLFunctions_3_3_Core>
#include <QImage>

namespace Qt_to_Gl
{
uint32_t GetFormat(QImage::Format in)
{
	switch(in)
	{
	case QImage::Format_Invalid:				return GL_NONE;
	case QImage::Format_Mono:					return GL_NONE;
	case QImage::Format_MonoLSB:				return GL_NONE;
	case QImage::Format_Indexed8:				return GL_STENCIL;
	case QImage::Format_RGB32:					return GL_RGB;
	case QImage::Format_ARGB32:					return GL_BGRA;
	case QImage::Format_ARGB32_Premultiplied:	return GL_RGBA;
	case QImage::Format_RGB16:					return GL_RGB;
	case QImage::Format_ARGB8565_Premultiplied:	return GL_BGRA;
	case QImage::Format_RGB666:					return GL_RGB;
	case QImage::Format_ARGB6666_Premultiplied:	return GL_BGRA;
	case QImage::Format_RGB555:					return GL_RGB;
	case QImage::Format_ARGB8555_Premultiplied:	return GL_BGRA;
	case QImage::Format_RGB888:					return GL_RGB;
	case QImage::Format_RGB444:					return GL_RGB;
	case QImage::Format_ARGB4444_Premultiplied:	return GL_BGRA;
	case QImage::Format_RGBX8888:				return GL_RGB;
	case QImage::Format_RGBA8888:				return GL_RGBA;
	case QImage::Format_RGBA8888_Premultiplied: return GL_RGBA;
	case QImage::Format_BGR30:					return GL_RGB;
	case QImage::Format_A2BGR30_Premultiplied:	return GL_RGBA;
	case QImage::Format_RGB30:					return GL_BGR;
	case QImage::Format_A2RGB30_Premultiplied:	return GL_BGRA;
	case QImage::Format_Alpha8:					return GL_RED;
	case QImage::Format_Grayscale8:				return GL_RED;
	case QImage::Format_RGBX64:					return GL_RGBA;
	case QImage::Format_RGBA64:					return GL_RGBA;
	case QImage::Format_RGBA64_Premultiplied:	return GL_RGBA;

#if QT_VERSION  >= QT_VERSION_CHECK(5, 14, 0)
	case QImage::Format_Grayscale16:			return GL_RED;
	case QImage::Format_BGR888:					return GL_BGR;
#endif
	default:
		throw std::logic_error("Unknown QImage::Format");
	}

	return GL_NONE;
}

uint32_t GetType(QImage::Format in)
{
	switch(in)
	{
	case QImage::Format_Invalid:				return GL_NONE;
	case QImage::Format_Mono:					return GL_NONE;
	case QImage::Format_MonoLSB:				return GL_NONE;
	case QImage::Format_Indexed8:				return GL_UNSIGNED_BYTE;
	case QImage::Format_RGB32:					return GL_UNSIGNED_INT_8_8_8_8_REV;
	case QImage::Format_ARGB32:					return GL_UNSIGNED_INT_8_8_8_8_REV;
	case QImage::Format_ARGB32_Premultiplied:	return GL_UNSIGNED_INT_8_8_8_8_REV;
	case QImage::Format_RGB16:					return GL_UNSIGNED_SHORT;
	case QImage::Format_ARGB8565_Premultiplied:	return GL_NONE;
	case QImage::Format_RGB666:					return GL_NONE;
	case QImage::Format_ARGB6666_Premultiplied:	return GL_NONE;
	case QImage::Format_RGB555:					return GL_RGB5;
	case QImage::Format_ARGB8555_Premultiplied:	return GL_NONE;
	case QImage::Format_RGB888:					return GL_UNSIGNED_BYTE;
	case QImage::Format_RGB444:					return GL_UNSIGNED_SHORT_4_4_4_4_REV;
	case QImage::Format_ARGB4444_Premultiplied:	return GL_UNSIGNED_SHORT_4_4_4_4_REV;
	case QImage::Format_RGBX8888:				return GL_UNSIGNED_INT_8_8_8_8;
	case QImage::Format_RGBA8888:				return GL_UNSIGNED_INT_8_8_8_8;
	case QImage::Format_RGBA8888_Premultiplied: return GL_UNSIGNED_INT_8_8_8_8;
	case QImage::Format_BGR30:					return GL_UNSIGNED_INT_2_10_10_10_REV;
	case QImage::Format_A2BGR30_Premultiplied:	return GL_UNSIGNED_INT_2_10_10_10_REV;
	case QImage::Format_RGB30:					return GL_UNSIGNED_INT_10_10_10_2;
	case QImage::Format_A2RGB30_Premultiplied:	return GL_UNSIGNED_INT_10_10_10_2;
	case QImage::Format_Alpha8:					return GL_UNSIGNED_BYTE;
	case QImage::Format_Grayscale8:				return GL_UNSIGNED_BYTE;
	case QImage::Format_RGBX64:					return GL_UNSIGNED_SHORT;
	case QImage::Format_RGBA64:					return GL_UNSIGNED_SHORT;
    case QImage::Format_RGBA64_Premultiplied:	return GL_UNSIGNED_SHORT;
#if QT_VERSION  >= QT_VERSION_CHECK(5, 14, 0)
	case QImage::Format_Grayscale16:			return GL_UNSIGNED_SHORT;
	case QImage::Format_BGR888:					return GL_UNSIGNED_BYTE;
#endif
	default:
		throw std::logic_error("Unknown QImage::Format");
	}

	return GL_NONE;
}


uint32_t GetInternalFormat(QImage::Format in)
{
	switch(in)
	{
	case QImage::Format_Invalid:				return GL_NONE;
	case QImage::Format_Mono:					return GL_NONE;
	case QImage::Format_MonoLSB:				return GL_NONE;
	case QImage::Format_Indexed8:				return GL_NONE;
	case QImage::Format_RGB32:					return GL_RGB8;
	case QImage::Format_ARGB32:					return GL_RGBA8;
	case QImage::Format_ARGB32_Premultiplied:	return GL_RGBA8;
	case QImage::Format_RGB16:					return GL_RGB16;
	case QImage::Format_ARGB8565_Premultiplied:	return GL_NONE;
	case QImage::Format_RGB666:					return GL_NONE;
	case QImage::Format_ARGB6666_Premultiplied:	return GL_NONE;
	case QImage::Format_RGB555:					return GL_RGB5;
	case QImage::Format_ARGB8555_Premultiplied:	return GL_NONE;
	case QImage::Format_RGB888:					return GL_RGB8;
	case QImage::Format_RGB444:					return GL_RGB4;
	case QImage::Format_ARGB4444_Premultiplied:	return GL_RGBA4;
	case QImage::Format_RGBX8888:				return GL_RGB8;
	case QImage::Format_RGBA8888:				return GL_RGBA8;
	case QImage::Format_RGBA8888_Premultiplied: return GL_RGBA8;
	case QImage::Format_BGR30:					return GL_RGB10;
	case QImage::Format_A2BGR30_Premultiplied:	return GL_RGB10_A2;
	case QImage::Format_RGB30:					return GL_RGB10;
	case QImage::Format_A2RGB30_Premultiplied:	return GL_RGB10_A2;
	case QImage::Format_Alpha8:					return GL_R8;
    case QImage::Format_Grayscale8:				return GL_R8;
	case QImage::Format_RGBX64:					return GL_RGB16;
	case QImage::Format_RGBA64:					return GL_RGBA16;
    case QImage::Format_RGBA64_Premultiplied:	return GL_RGBA16;

#if QT_VERSION  >= QT_VERSION_CHECK(5, 14, 0)
    case QImage::Format_Grayscale16:			return GL_R16;
	case QImage::Format_BGR888:					return GL_RGB8;
#endif
	default:
		throw std::logic_error("Unknown QImage::Format");
	}

	return GL_NONE;
}

bool           ImageUsesAlpha(const QImage & in)
{
	if(!in.hasAlphaChannel())
		return false;

	for(auto y = 0; y < in.height(); ++y)
	{
		for(auto x = 0; x < in.width(); ++x)
		{
			if(qAlpha(in.pixel(x,y)) != 255)
				return true;
		}
	}

	return false;
}

QImage::Format GetTargetFormat(QImage const& in)
{
	switch(in.format())
	{
	case QImage::Format_Invalid:				return QImage::Format_Invalid;
	case QImage::Format_Mono:					return QImage::Format_Grayscale8;
	case QImage::Format_MonoLSB:				return QImage::Format_Grayscale8;
	case QImage::Format_Indexed8:				return QImage::Format_ARGB32;
	case QImage::Format_RGB32:					return QImage::Format_RGB32;
	case QImage::Format_ARGB32:					return QImage::Format_ARGB32;
	case QImage::Format_ARGB32_Premultiplied:	return QImage::Format_ARGB32_Premultiplied;
	case QImage::Format_RGB16:					return QImage::Format_RGB16;
	case QImage::Format_ARGB8565_Premultiplied:	break;
	case QImage::Format_RGB666:					return QImage::Format_ARGB32;
	case QImage::Format_ARGB6666_Premultiplied:	break;
	case QImage::Format_RGB555:					return QImage::Format_RGB555;
	case QImage::Format_ARGB8555_Premultiplied:	break;
	case QImage::Format_RGB888:					return QImage::Format_RGB888;
	case QImage::Format_RGB444:					return QImage::Format_RGB444;
	case QImage::Format_ARGB4444_Premultiplied: return QImage::Format_ARGB4444_Premultiplied;
	case QImage::Format_RGBX8888:				return QImage::Format_RGBX8888;
	case QImage::Format_RGBA8888:				return QImage::Format_RGBA8888;
	case QImage::Format_RGBA8888_Premultiplied:	return QImage::Format_RGBA8888_Premultiplied;
	case QImage::Format_BGR30:					return QImage::Format_BGR30;
	case QImage::Format_A2BGR30_Premultiplied:	return QImage::Format_A2BGR30_Premultiplied;
	case QImage::Format_RGB30:					return QImage::Format_RGB30;
	case QImage::Format_A2RGB30_Premultiplied:	return QImage::Format_A2RGB30_Premultiplied;
	case QImage::Format_Alpha8:					return QImage::Format_Alpha8;
	case QImage::Format_Grayscale8:				return QImage::Format_Grayscale8;
	case QImage::Format_RGBX64:					return QImage::Format_RGBX64;
	case QImage::Format_RGBA64:					return QImage::Format_RGBA64;
	case QImage::Format_RGBA64_Premultiplied:	return QImage::Format_RGBA64_Premultiplied;
#if QT_VERSION  >= QT_VERSION_CHECK(5, 14, 0)
	case QImage::Format_Grayscale16:			return QImage::Format_Grayscale16;
	case QImage::Format_BGR888:					return QImage::Format_BGR888;
#endif
	default:
		throw std::logic_error("Unknown QImage::Format");
	}

	for(auto y = 0; y < in.height(); ++y)
	{
		for(auto x = 0; x < in.width(); ++x)
		{
			if(QColor(in.pixel(x,y)).alpha() != 255)
				return QImage::Format_ARGB32;
		}
	}

	return QImage::Format_RGB888;
}

uint32_t GetFormatFromInternalFormat(uint32_t in)
{
	switch(in)
	{
	case GL_RED: 			return GL_RED;
	case GL_RG: 			return GL_RG;
	case GL_RGB:			return GL_RGB;
	case GL_RGBA:			return GL_RGBA;
	case GL_RED_INTEGER: 	return GL_RED_INTEGER;
	case GL_RG_INTEGER:		return GL_RG_INTEGER;
	case GL_RGB_INTEGER:	return GL_RGB_INTEGER;
	case GL_RGBA_INTEGER: 	return GL_RGBA_INTEGER;

	case GL_R8: 			return GL_RED;
	case GL_R8_SNORM: 		return GL_RED;
	case GL_R16: 			return GL_RED;
	case GL_R16_SNORM: 		return GL_RED;
	case GL_RG8: 			return GL_RG;
	case GL_RG8_SNORM:	 	return GL_RG;
	case GL_RG16: 			return GL_RG;
	case GL_RG16_SNORM: 	return GL_RG;
	case GL_R3_G3_B2: 		return GL_RGB;
	case GL_RGB4: 			return GL_RGB;
	case GL_RGB5: 			return GL_RGB;
	case GL_RGB8: 			return GL_RGB;
	case GL_RGB8_SNORM: 	return GL_RGB;
	case GL_RGB10: 			return GL_RGB;
	case GL_RGB12: 			return GL_RGB;
	case GL_RGB16_SNORM: 	return GL_RGB;
	case GL_RGBA2: 			return GL_RGB;
	case GL_RGBA4: 			return GL_RGB;
	case GL_RGB5_A1: 		return GL_RGBA;
	case GL_RGBA8: 			return GL_RGBA;
	case GL_RGBA8_SNORM: 	return GL_RGBA;
	case GL_RGB10_A2: 		return GL_RGBA;
	case GL_RGB10_A2UI: 	return GL_RGBA_INTEGER;
	case GL_RGBA12: 		return GL_RGBA;
	case GL_RGBA16:			return GL_RGBA;
	case GL_SRGB8:			return GL_RGB;
	case GL_SRGB8_ALPHA8:	return GL_RGBA;
	case GL_R16F:			return GL_RED;
	case GL_RG16F:			return GL_RG;
	case GL_RGB16F:			return GL_RGB;
	case GL_RGBA16F:		return GL_RGBA;
	case GL_R32F:			return GL_RED;
	case GL_RG32F:			return GL_RG;
	case GL_RGB32F:			return GL_RGB;
	case GL_RGBA32F:		return GL_RGBA;
	case GL_R11F_G11F_B10F:	return GL_RGB;
	case GL_RGB9_E5:		return GL_RGB;
	case GL_R8I:			return GL_RED_INTEGER;
	case GL_R8UI:			return GL_RED_INTEGER;
	case GL_R16I:			return GL_RED_INTEGER;
	case GL_R16UI:			return GL_RED_INTEGER;
	case GL_R32I:			return GL_RED_INTEGER;
	case GL_R32UI:			return GL_RED_INTEGER;
	case GL_RG8I:			return GL_RG_INTEGER;
	case GL_RG8UI:			return GL_RG_INTEGER;
	case GL_RG16I:			return GL_RG_INTEGER;
	case GL_RG16UI:			return GL_RG_INTEGER;
	case GL_RG32I:			return GL_RG_INTEGER;
	case GL_RG32UI:			return GL_RG_INTEGER;
	case GL_RGB8I:			return GL_RGB_INTEGER;
	case GL_RGB8UI:			return GL_RGB_INTEGER;
	case GL_RGB16I:			return GL_RGB_INTEGER;
	case GL_RGB16UI:		return GL_RGB_INTEGER;
	case GL_RGB32I:			return GL_RGB_INTEGER;
	case GL_RGB32UI:		return GL_RGB_INTEGER;
	case GL_RGBA8I:			return GL_RGBA_INTEGER;
	case GL_RGBA8UI:		return GL_RGBA_INTEGER;
	case GL_RGBA16I:		return GL_RGBA_INTEGER;
	case GL_RGBA16UI:		return GL_RGBA_INTEGER;
	case GL_RGBA32I:		return GL_RGBA_INTEGER;
	case GL_RGBA32UI:		return GL_RGBA_INTEGER;
	case GL_COMPRESSED_RED: 					return GL_RED;
	case GL_COMPRESSED_RG: 						return GL_RG;
	case GL_COMPRESSED_RGB: 					return GL_RGB;
	case GL_COMPRESSED_RGBA: 					return GL_RGBA;
	case GL_COMPRESSED_SRGB: 					return GL_RGB;
	case GL_COMPRESSED_SRGB_ALPHA: 				return GL_RGBA;
	case GL_COMPRESSED_RED_RGTC1: 				return GL_RED;
	case GL_COMPRESSED_SIGNED_RED_RGTC1: 		return GL_RED;
	case GL_COMPRESSED_RG_RGTC2: 				return GL_RG;
	case GL_COMPRESSED_SIGNED_RG_RGTC2: 		return GL_RG;
	case GL_COMPRESSED_RGBA_BPTC_UNORM: 		return GL_RGBA;
	case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM: 	return GL_RGBA;
	case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT: 	return GL_RGB;
	case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT: return GL_RGB;
	default:
		break;
	}

	return GL_NONE;
}

uint32_t GetTypeFromInternalFormat(uint32_t in)
{
	switch(in)
	{
	case GL_R8:				return GL_UNSIGNED_BYTE;
	case GL_R8_SNORM:		return GL_UNSIGNED_BYTE;
	case GL_R16:			return GL_UNSIGNED_SHORT;
	case GL_R16_SNORM:		return GL_UNSIGNED_SHORT;
	case GL_RG8:			return GL_UNSIGNED_BYTE;
	case GL_RG8_SNORM:		return GL_UNSIGNED_BYTE;
	case GL_RG16:			return GL_UNSIGNED_SHORT;
	case GL_RG16_SNORM:		return GL_UNSIGNED_SHORT;
	case GL_R3_G3_B2:		return GL_UNSIGNED_BYTE_3_3_2;
	case GL_RGB4:			return GL_UNSIGNED_SHORT_4_4_4_4;
	case GL_RGB5:			return GL_UNSIGNED_SHORT_5_6_5;
	case GL_RGB8:			return GL_UNSIGNED_BYTE;
	case GL_RGB8_SNORM:		return GL_UNSIGNED_BYTE;
	case GL_RGB10:			return GL_UNSIGNED_INT_10_10_10_2;
	case GL_RGB12:			return GL_UNSIGNED_SHORT;
	case GL_RGB16_SNORM:	return GL_UNSIGNED_SHORT;
	case GL_RGBA2:			return GL_UNSIGNED_SHORT_4_4_4_4;
	case GL_RGBA4:			return GL_UNSIGNED_SHORT_4_4_4_4;
	case GL_RGB5_A1:		return GL_UNSIGNED_SHORT_5_5_5_1;
	case GL_RGBA8:			return GL_UNSIGNED_BYTE;
	case GL_RGBA8_SNORM:	return GL_UNSIGNED_BYTE;
	case GL_RGB10_A2:		return GL_UNSIGNED_INT_10_10_10_2;
	case GL_RGB10_A2UI:		return GL_UNSIGNED_INT_10_10_10_2;
	case GL_RGBA12:			return GL_UNSIGNED_SHORT;
	case GL_RGBA16:			return GL_UNSIGNED_SHORT;
	case GL_SRGB8:			return GL_UNSIGNED_BYTE;
	case GL_SRGB8_ALPHA8:	return GL_UNSIGNED_BYTE;
	case GL_R16F:			return GL_HALF_FLOAT;
	case GL_RG16F:			return GL_HALF_FLOAT;
	case GL_RGB16F:			return GL_HALF_FLOAT;
	case GL_RGBA16F:		return GL_HALF_FLOAT;
	case GL_R32F:			return GL_FLOAT;
	case GL_RG32F:			return GL_FLOAT;
	case GL_RGB32F:			return GL_FLOAT;
	case GL_RGBA32F:		return GL_FLOAT;
	case GL_R11F_G11F_B10F:	return GL_UNSIGNED_INT_10_10_10_2;
	case GL_RGB9_E5:		return GL_UNSIGNED_INT_10_10_10_2;
	case GL_R8I:			return GL_BYTE;
	case GL_R8UI:			return GL_UNSIGNED_BYTE;
	case GL_R16I:			return GL_SHORT;
	case GL_R16UI:			return GL_UNSIGNED_SHORT;
	case GL_R32I:			return GL_INT;
	case GL_R32UI:			return GL_UNSIGNED_INT;
	case GL_RG8I:			return GL_BYTE;
	case GL_RG8UI:			return GL_UNSIGNED_BYTE;
	case GL_RG16I:			return GL_SHORT;
	case GL_RG16UI:			return GL_UNSIGNED_SHORT;
	case GL_RG32I:			return GL_INT;
	case GL_RG32UI:			return GL_UNSIGNED_INT;
	case GL_RGB8I:			return GL_BYTE;
	case GL_RGB8UI:			return GL_UNSIGNED_BYTE;
	case GL_RGB16I:			return GL_SHORT;
	case GL_RGB16UI:		return GL_UNSIGNED_SHORT;
	case GL_RGB32I:			return GL_INT;
	case GL_RGB32UI:		return GL_UNSIGNED_INT;
	case GL_RGBA8I:			return GL_BYTE;
	case GL_RGBA8UI:		return GL_UNSIGNED_BYTE;
	case GL_RGBA16I:		return GL_SHORT;
	case GL_RGBA16UI:		return GL_UNSIGNED_SHORT;
	case GL_RGBA32I:		return GL_INT;
	case GL_RGBA32UI:		return GL_UNSIGNED_INT;
	default:
		break;
	}

	return GL_NONE;
}

uint32_t GetPixelByteWidth(uint32_t format, uint32_t type)
{
	uint32_t width{};

	switch(type)
	{
	case GL_UNSIGNED_BYTE:	width = 1; break;
	case GL_BYTE:			width = 1; break;
	case GL_UNSIGNED_SHORT:	width = 2; break;
	case GL_SHORT:			width = 2; break;
	case GL_UNSIGNED_INT:	width = 4; break;
	case GL_INT:			width = 4; break;
	case GL_HALF_FLOAT:		width = 2; break;
	case GL_FLOAT:			width = 4; break;

	case GL_UNSIGNED_BYTE_3_3_2:			return 1;
	case GL_UNSIGNED_BYTE_2_3_3_REV:		return 1;
	case GL_UNSIGNED_SHORT_5_6_5:			return 2;
	case GL_UNSIGNED_SHORT_5_6_5_REV:		return 2;
	case GL_UNSIGNED_SHORT_4_4_4_4:			return 2;
	case GL_UNSIGNED_SHORT_4_4_4_4_REV:		return 2;
	case GL_UNSIGNED_SHORT_5_5_5_1:			return 2;
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:		return 2;
	case GL_UNSIGNED_INT_8_8_8_8:			return 4;
	case GL_UNSIGNED_INT_8_8_8_8_REV:		return 4;
	case GL_UNSIGNED_INT_10_10_10_2:		return 4;
	case GL_UNSIGNED_INT_2_10_10_10_REV:	return 4;
	case GL_UNSIGNED_INT_24_8:				return 4;
	case GL_UNSIGNED_INT_10F_11F_11F_REV:	return 4;
	case GL_UNSIGNED_INT_5_9_9_9_REV:		return 4;
	case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:	return 4;
	default:
		throw std::logic_error("Unknown GL-type");

	}

	return GetChannelsFromFormat(format) * width;
}

uint32_t GetChannelsFromFormat(uint32_t format)
{
	switch(format)
	{
	case GL_STENCIL_INDEX:		return 1;
	case GL_DEPTH_COMPONENT:	return 1;
	case GL_DEPTH_STENCIL:		return 1;
	case GL_RED:				return 1;
	case GL_GREEN:				return 1;
	case GL_BLUE:				return 1;
	case GL_RG:					return 2;
	case GL_RGB:				return 3;
	case GL_RGBA:				return 4;
	case GL_BGR:				return 3;
	case GL_BGRA:				return 4;
	case GL_RED_INTEGER:		return 1;
	case GL_GREEN_INTEGER:		return 1;
	case GL_BLUE_INTEGER:		return 1;
	case GL_RG_INTEGER:			return 2;
	case GL_RGB_INTEGER:		return 3;
	case GL_RGBA_INTEGER:		return 4;
	case GL_BGR_INTEGER:		return 3;
	case GL_BGRA_INTEGER:		return 4;

	default:
		throw std::logic_error("Unknown GL-format");
	}

	return 1;
}

bool HasAlpha(uint32_t in)
{
	switch(in)
	{
	case GL_RGB5_A1: 		return true;
	case GL_RGBA8: 			return true;
	case GL_RGBA8_SNORM: 	return true;
	case GL_RGB10_A2: 		return true;
	case GL_RGB10_A2UI: 	return true;
	case GL_RGBA12: 		return true;
	case GL_RGBA16:			return true;
	case GL_SRGB8_ALPHA8:	return true;
	case GL_RGBA16F:		return true;
	case GL_RGBA32F:		return true;
	case GL_RGBA8I:			return true;
	case GL_RGBA8UI:		return true;
	case GL_RGBA16I:		return true;
	case GL_RGBA16UI:		return true;
	case GL_RGBA32I:		return true;
	case GL_RGBA32UI:		return true;
	case GL_COMPRESSED_RGBA: 					return true;
	case GL_COMPRESSED_SRGB_ALPHA: 				return true;
	case GL_COMPRESSED_RGBA_BPTC_UNORM: 		return true;
	case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM: 	return true;
	default:
		break;
	}

	return false;
}

}
