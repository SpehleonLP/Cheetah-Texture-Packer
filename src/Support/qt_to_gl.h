#ifndef QT_TO_GL_H
#define QT_TO_GL_H
#include <QImage>

namespace Qt_to_Gl
{

QImage::Format GetTargetFormat(const QImage & in);
bool           ImageUsesAlpha(const QImage & in);

uint32_t GetInternalFormat(QImage::Format in);
uint32_t GetFormat(QImage::Format in);
uint32_t GetType(QImage::Format in);

uint32_t GetFormatFromInternalFormat(uint32_t);
uint32_t GetTypeFromInternalFormat(uint32_t);
uint32_t GetPixelByteWidth(uint32_t format, uint32_t type);

uint32_t GetChannelsFromFormat(uint32_t format);
bool     HasAlpha(uint32_t format);

}


#endif // QT_TO_GL_H
