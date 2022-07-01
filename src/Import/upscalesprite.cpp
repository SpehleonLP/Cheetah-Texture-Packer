#include "upscalesprite.h"
#include "import_c16.h"
#include "super_xbr.h"
#include "widgets/glviewwidget.h"
#include <stdexcept>
#include <QProgressDialog>
#include <QCoreApplication>
#include <vector>

static
void xBrLoop(std::vector<uint32_t> & pixel_data, int & width, int & height)
{
	std::vector<uint32_t> output;
	output.resize((width*2)*(height*2));

	scaleSuperXbr(pixel_data.data(), output.data(), width, height);

	width *= 2;
	height *= 2;
	pixel_data = output;
}


SpriteFile double_image(SpriteFile const& image, QWidget * parent)
{
	if(image.format != GL_RGBA
	|| image.type   != GL_UNSIGNED_BYTE)
		throw std::logic_error("improper image format for superXbr Scaling");

//alloc 2x the size of the original
	SpriteFile r(image);

	r.sizes    = CountedSizedArray<glm::u16vec2>(r.count);
	r.pointers = CountedSizedArray<void*>(r.count);

	uint32_t size = 0;

	for(uint32_t i = 0; i < r.count; ++i)
	{
		r.sizes[i] = image.sizes[i]*(uint16_t)2;
		size += r.sizes[i].x * r.sizes[i].y;
	}

	r.heap = CountedSizedArray<uint8_t>(size*4);

	size = 0;
	for(uint32_t i = 0; i < r.count; ++i)
	{
		r.pointers[i] = &r.heap[4 * size];
		size += r.sizes[i].x * r.sizes[i].y;
	}

	QProgressDialog dialog("Upscaling Sprite...", "Cannot Cancel", 0, r.count, parent);

	for(uint32_t i = 0; i < r.count; ++i)
	{
		dialog.setValue(i+1);
		scaleSuperXbr((uint32_t*)image.pointers[i], (uint32_t*)r.pointers[i], image.sizes[i].x, image.sizes[i].y);
		QCoreApplication::processEvents();
	}

	dialog.close();

	return r;
}
