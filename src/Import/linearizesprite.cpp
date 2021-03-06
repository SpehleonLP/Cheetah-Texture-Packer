#include "linearizesprite.h"
#include "import_c16.h"
#include "Support/qt_to_gl.h"
#include "widgets/glviewwidget.h"

SpriteFile Linearize::ToFloat(SpriteFile const& spr)
{
	assert(spr.type == GL_UNSIGNED_BYTE);

	SpriteFile r(spr);

	r.type = GL_FLOAT;
	r.AllocHeap();

	float * dst = (float*)&r.heap[0];
	const uint8_t * src = &spr.heap[0];

	int channels = Qt_to_Gl::GetChannelsFromFormat(spr.format);
	size_t N     = r.GetTotalPixels() * channels;

	for(size_t i = 0; i < N; ++i)
	{
		dst[i]  = (src[i] / 255.f);
		if((i % channels) != 3)	dst[i] *= dst[i];
	}

	return r;
}

SpriteFile Linearize::ToUByte(SpriteFile const& spr)
{
	assert(spr.type == GL_FLOAT);

	SpriteFile r(spr);

	r.type = GL_UNSIGNED_BYTE;
	r.AllocHeap();

	uint8_t * dst = &r.heap[0];
	const float * src = (float*) &spr.heap[0];

	int channels = Qt_to_Gl::GetChannelsFromFormat(spr.format);
	size_t N     = r.GetTotalPixels() * channels;

	for(size_t i = 0; i < N; ++i)
	{
		float eax = ((i % channels) != 3)? std::sqrt(src[i]) : src[i];
		dst[i] = std::max(0, std::min<int>(255, std::round(eax * 255.f)));
	}

	return r;
}

#if 0
#include "spritebuilder.h"
#include "ui_spritebuilder.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QFileInfo>
#include "byteswap.h"
#include "importsettings.h"
#include "imageview.h"
#include <cmath>

#define BAKED_IMAGE_SLOT 0

const static QRegExp validator("^[a-z][0-9]{2}(([a-z].[cs]16)|([0-9].spr))", Qt::CaseInsensitive);
QImage double_image(QImage image);

char getPartNumber(char part)
{
	switch(tolower(part))
	{
	default: return 0;
	case 'c':
	case 'f': return 1;
	case 'd':
	case 'g': return 2;
	case 'e':
	case 'h': return 2;

	case 'i':
	case 'k': return 1;

	case 'j':
	case 'l': return 0;

//tail sort of necessarily has to match body
	case 'm':
	case 'n':
	case 'b': return 2;

//ears/hair match head
	case 'o':
	case 'p':
	case 'q':
	case 'a': return 1;
		return 0;
	}

}

template<typename T>
static inline
T ALWAYS_INLINE sq(const T & t)
{
	return t*t;
}

template<int (*F)(QRgb)>
static inline
int FLATTEN ALWAYS_INLINE avg_internal(QRgb t)
{
	return F(t);
}

template<int (*F)(QRgb), typename... Args>
static inline
int FLATTEN ALWAYS_INLINE avg_internal(QRgb t, Args&&... args)
{
	return avg_internal<F>(t) + avg_internal<F>(std::forward<Args>(args)...);
}

template<int (*F)(QRgb), typename... Args>
static
int FLATTEN avg(Args&&... args)
{
	return avg_internal<F>(std::forward<Args>(args)...) / sizeof...(args);
}

template<int (*F)(QRgb)>
static inline
int FLATTEN ALWAYS_INLINE sq_internal(int & /*i*/, QRgb t)
{
	return sq(F(t));
}

template<int (*F)(QRgb), typename... Args>
static inline
int FLATTEN ALWAYS_INLINE sq_internal(int & i, QRgb t, Args&&... args)
{
	++i;
	return sq_internal<F>(i, t) + sq_internal<F>(i, std::forward<Args>(args)...);
}

template<int (*F)(QRgb), typename... Args>
static
int FLATTEN sq_s(Args&&... args)
{
	int i = 1;
	int j = sq_internal<F>(i, std::forward<Args>(args)...);
	return sqrt(j / i);
}

template<typename... Args>
static inline
QRgb blur(Args&&... args)
{
	return qRgba(sq_s<qRed>(std::forward<Args>(args)...), sq_s<qGreen>(std::forward<Args>(args)...), sq_s<qBlue>(std::forward<Args>(args)...), avg<qAlpha>(std::forward<Args>(args)...) * (sizeof...(args)) / 8);
}

void getAdjacentAlpha(QRgb pixel[9], const QImage & original, int x, int y)
{
	for(int8_t i = 0; i < 9; ++i)
	{
		pixel[i] = original.pixel(x, y);
	}

	int8_t i = 0;
	for(int8_t _x = -1; _x < 2; ++_x)
	{
		if(0 <= _x + x && _x + x < original.width())
		{
			i = (_x + 1)*3;

			for(int8_t _y = -1; _y < 2; ++_y, ++i)
			{
				if(0 <= _y + y && _y + y < original.height())
				{
					pixel[i] = original.pixel(x + _x, y + _y);
				}
			}
		}
	}
}

QRgb blurTransparentPixel(const QImage & original, int x, int y, bool force = false)
{
	QRgb c[9];
	getAdjacentAlpha(c, original, x, y);

	int red=0, green=0, blue=0, alpha=0;
	int j = 0;

	for(uint8_t i = 0; i < 9; ++i)
	{
		red	  += qRed(c[i])*qRed(c[i]);
		green += qGreen(c[i])*qGreen(c[i]);
		blue  += qBlue(c[i])*qBlue(c[i]);
		c[i]   = qAlpha(c[i]);
		alpha += c[i];

		if(c[i])
		{
			++j;
		}
	}

	if((force && j)
	|| (c[1] && c[7]) || (c[3] && c[5])
	||(((0 < c[1] && c[1] < 255)
	||  (0 < c[7] && c[7] < 255))
	&& ((0 < c[3] && c[3] < 255)
	||  (0 < c[5] && c[5] < 255))) // */

	)
	{
		return qRgba(
			sqrt(red/j),
			sqrt(green/j),
			sqrt(blue/j),
			alpha/9);
	}

	return original.pixel(x, y);

/*
	QRgb l = x   == 0				 ? 0 : original.pixel(x-1, y  );
	QRgb r = x+1 == original.width() ? 0 : original.pixel(x+1, y  );
	QRgb t = y   == 0				 ? 0 : original.pixel(x  , y-1);
	QRgb b = y+1 == original.height()? 0 : original.pixel(x  , y+1);

	int c = ((!!qAlpha(l)) << 12)
		  | ((!!qAlpha(r)) << 8)
		  | ((!!qAlpha(t)) << 4)
		  | ( !!qAlpha(b));

	switch(c)
	{
	default:
		break;
	case 0x0011: return blur(t, b);
	case 0x0111: return blur(r, t, b);
	case 0x1011: return blur(l, t, b);
	case 0x1100: return blur(l, r);
	case 0x1101: return blur(l, r, b);
	case 0x1110: return blur(l, r, t);
	case 0x1111: return blur(l, r, t, b);
	}

	c = c & ( ((qAlpha(l) < 255) << 12)
			| ((qAlpha(r) < 255) << 8)
			| ((qAlpha(t) < 255) << 4)
			|  (qAlpha(b) < 255));

	switch(c)
	{
	default:
		break;
	case 0x0101: return blur(r, b);
	case 0x0110: return blur(r, t);
	case 0x1001: return blur(l, b);
	case 0x1010: return blur(l, t);
	}

	return original.pixel(x, y);*/
}


void forceBlurTransparentPixel(const QImage & original, QImage & retn, const int x, const int y)
{
	if(x-1 >= 0 && x+1 <= original.width()
	&& y-1 >= 0 && y+1 <= original.height())
	{
		retn.setPixel(x, y, blurTransparentPixel(original, x, y, true));
	}
}

QRgb blurSolidPixel(const QImage & original,  QImage & retn, const int x, const int y)
{
	QRgb c[9];
	getAdjacentAlpha(c, original, x, y);

	uint32_t j = 0;
	for(uint8_t i = 0; i < 9; ++i)
	{
		c[i] = qAlpha(c[i]);
		j += c[i];
	}

	if((!c[1] && !c[7]) || (!c[3] && !c[5]))
	{
//surrounded by transparency, must be in very loose dither...
		if(j == c[4])
		{
			forceBlurTransparentPixel(original, retn, x-1, y-1);
			forceBlurTransparentPixel(original, retn, x  , y-1);
			forceBlurTransparentPixel(original, retn, x+1, y-1);
			forceBlurTransparentPixel(original, retn, x-1, y  );

			forceBlurTransparentPixel(original, retn, x+1, y  );
			forceBlurTransparentPixel(original, retn, x-1, y+1);
			forceBlurTransparentPixel(original, retn, x  , y+1);
			forceBlurTransparentPixel(original, retn, x+1, y+1);
		}

		return j / 16;
	}

	return c[4];
}

int blur_colors(QImage & original)
{
	QImage retn(original);

	int no_changed = 0;

	for(int x = 0; x < original.width(); ++x)
	{
		for(int y = 0; y < original.height(); ++y)
		{
			QRgb c = original.pixel(x, y);

			if(qAlpha(c) == 0)
			{
				auto t = blurTransparentPixel(original, x, y);
				if(t != c)
				{
					retn.setPixel(x, y, t);
					no_changed += 1;
				}
			}
			else if(qAlpha(c) == 0xFF)
			{
				int t = blurSolidPixel(original, retn, x, y);
				if(t != qAlpha(c))
				{
					retn.setPixel(x, y, (t << 24) | (c & 0x00FFFFFF));
					no_changed += 1;
				}
			}
		}
	}

	original = retn;
	return no_changed;
}

QImage blur_colors(const QImage & original, int blur_iterations)
{
	QImage retn(original);
	for(int i = 0; i < blur_iterations; ++i)
	{
		if(blur_colors(retn) == 0)
		{
			break;
		}
	}
	return retn;
}

int blur_alpha(QImage & image)
{
	QImage retn(image);

	int no_changed = 0;

		int d[4];
		for(int x = 0; x < image.width(); ++x)
		{
			d[3] = x     ==			    0? 0 : x-1;
			d[2] = x + 1 == image.width()? 0 : x+1;

			for(int y = 0; y < image.height(); ++y)
			{
				d[1] = y     ==			     0? 0 : y-1;
				d[0] = y + 1 == image.height()? 0 : y+1;

				int o = qAlpha(image.pixel(x, y));

				if(o == 0)
				{
					continue;
				}

				QRgb c[9];
				c[0] = qAlpha(image.pixel(d[3], d[1]));
				c[1] = qAlpha(image.pixel(d[3], y  ));
				c[2] = qAlpha(image.pixel(d[3], d[0]));
				c[3] = qAlpha(image.pixel(x  , d[1]));
				c[4] = qAlpha(image.pixel(x, y  ));
				c[5] = qAlpha(image.pixel(x, d[0]));
				c[6] = qAlpha(image.pixel(d[2], d[1]));
				c[7] = qAlpha(image.pixel(d[2], y  ));
				c[8] = qAlpha(image.pixel(d[2], d[0]));

/*
				if(o == 255)
				{
					int t = 0;
					for(int i = 0; i < 9; ++i)
					{
						t += (c[i] == 0 || c[i] == 255);
					}

					if(t > 6)
					{
						continue;
					}
				} // */


				int p = 0;
				int j = 0;
				for(int i = 0; i < 9; ++i)
				{
					if(0 < c[i])
					{
						p += c[i];
						++j;
					}
				}


				if(p && j)
				{
					p = p / j;
					if(p != o)
					{
						retn.setPixel(x, y, (p << 24) | (image.pixel(x, y) & 0x00FFFFFF));
						++no_changed;
					}
				}
			}
		}

	image = retn;
	return no_changed;
}

QImage blur_alpha(const QImage & original, int blur_iterations)
{
	QImage retn(original);
	for(int i = 0; i < blur_iterations; ++i)
	{
		if(!blur_alpha(retn))
		{
			break;
		}
	}
	return retn;
}

#include <glm/glm.hpp>

inline
unsigned int UNUSED ChromaKey(WORD color)
{
	glm::vec3 chroma = glm::vec3(1, 0, 1);

	int r = ((color >> 8)) & 0xFF;
	int g = ((color >> 3)) & 0xFF;
	int b = color << 3 & 0xFF;

	glm::vec3  c(r, g, b);
	c /= 255.f;

	float whiteness = c.x * c.y * c.z;

//	c = glm::pow(c, glm::vec3(1 / 2.2));

	auto length = glm::length(c);
	c /= length;

//	c = c * 2.f - 1.f;

//get level of purple
//	float dot = glm::dot(c, chroma) * (1 - whiteness);
//	c = c - dot * chroma;

//	c = 1.f - c;

//	c = (c + 1.f) * .5f;

//	c = glm::pow(c, glm::vec3(2.2));

	c *= length;

	c = glm::vec3(length * 1.5 - .5);
	//c = glm::clamp(c, glm::vec3(0), glm::vec3(1));

	r = std::ceil(c.r * 255);
	g = std::ceil(c.g * 255);
	b = std::ceil(c.b * 255);

	return (((((0xFF << 8) | r) << 8) | g) << 8) | b;
}

void SpriteBuilder::documentImport()
{
	if(!documentPreClose())
	{
		return;
	}

	QString name = QFileDialog::getOpenFileName(this, tr("Open Creatures Sprite"), QString(), tr("Creatures Sprite (*.s16 *.c16)"));

	if(name.isEmpty())
	{
		return;
	}


	FILE * file = fopen(name.toStdString().c_str(), "rb");

	if(!file)
	{
		QMessageBox mesg;
		mesg.setText(QObject::tr("Unable to open file '%1' for reading.").arg(filename));
		mesg.exec();
		documentImport();
		return;
	}

	int bit_flags;

	fread(&bit_flags, 4, 1, file);
	bit_flags = byte_swap(bit_flags);

	if(bit_flags > 3)
	{
		QMessageBox mesg;
		mesg.setText(QObject::tr("The header of file '%1' is corrupt.").arg(filename));
		mesg.exec();
		documentImport();
		return;
	}

	bool _565 = bit_flags & 0x01;
	bool _c16 = bit_flags & 0x02;

	m_settings.Update(this);

	if(!m_settings.accepted)
	{
		fclose(file);
		return;
	}

	ui->tableWidget->clearContents();
	ui->tableWidget->setRowCount(0);
	autosave_timer.stop();
	imported = false;

	filename = name;
	ui->tableWidget->command_list.clear();

	short no_images;
	fread(&no_images, sizeof(no_images), 1, file);

	int import_time = m_settings.import_time*no_images;

	name = QFileInfo(filename ).fileName();
	bool is_creature_sprite = validator.exactMatch(name);
	char body_part = tolower(name.at(0).toLatin1());
	uint8_t part = getPartNumber(body_part);

	QProgressDialog progress(tr("Importing Sprite '%1'").arg(name), "Abort Import", 0, import_time, this);
	progress.setMinimumDuration(0);
	progress.show();

	bool skip = false;
	uint32_t offset;
	uint16_t width;
	uint16_t height;

	ui->tableWidget->selectColumn(3);
	for(int i = 0; i < no_images; ++i)
	{
		if(_c16 && skip)
		{
			fseek(file, (height-1) * sizeof(uint32_t), SEEK_CUR);
		}
		skip = true;

		int prog = i*m_settings.import_time;
		progress.setValue(prog);
		if(prog & 0x01) qApp->processEvents();

		if(progress.wasCanceled())
		{
			break;
		}

		fread(&offset, sizeof(offset), 1, file);
		fread(&width, sizeof(width), 1, file);
		fread(&height, sizeof(height), 1, file);

		width = byte_swap(width);
		height = byte_swap(height);

		if(is_creature_sprite)
		{
			if(m_settings.eliminate_unnecessary)
			{
//creatures 2 sprite
				if(no_images == 120
				|| no_images == 10)
				{
					int c = i % 10;

					if(c != 8
					&& c != 9
					&& c != part
					&& c != part+4)
					{
						continue;
					}
				}
				else if(no_images % 16 == 0)
				{
					if(i % 4 != part)
					{
						continue;
					}
				}
			}
		}

		ui->tableWidget->insertRow(ui->tableWidget->rowCount());

		if(is_creature_sprite)
		{
			if(m_settings.bilaterally_symmetrical)
			{
				if(body_part == 'b'
				|| body_part >= 'm'
				|| body_part == 'a')
				{
					if(no_images % 16 == 0)
					{
						if(i % 16 < 4)
						{
							continue;
						}
					}
					else if(no_images % 10 == 0)
					{
						if(i % 10 < 4)
						{
							continue;
						}
					}
				}
			}

			if(m_settings.eliminate_reverse_blink)
			{
				if(body_part == 'a')
				{
					if(no_images == 120)
					{
						if(i % 20 == 19)
						{
							continue;
						}
					}
					else
					{
						if(i % 32 >= 28)
						{
							continue;
						}
					}
				}
				else if(body_part == 'b' && no_images > 16)
				{
					if(i > 16 && i % 16 >= 12)
					{
						continue;
					}
				}
			}

		}


		QImage image(
			m_settings.resize? ((width +1) & 0xFFFE) : ((width +3) & 0xFFFE),
			m_settings.resize? ((height+1) & 0xFFFE) : ((height+3) & 0xFFFC), QImage::Format_ARGB32);

		image.fill(0);

		if(_c16)
		{
			std::vector<uint32_t> offsets;
			offsets.resize(height);
			offsets[0] = offset;
			fread(offsets.data()+1, sizeof(uint32_t), height-1, file);

			fpos_t current_pos;
			fgetpos(file, &current_pos);

			for(int i = 0; i < height; ++i)
			{
				offset = byte_swap(offsets[i]);

				fseek(file, offset, SEEK_SET);

				for(int j = 0; j < width; )
				{
					uint16_t tag;
					fread(&tag, sizeof(tag), 1, file);
					tag = byte_swap(tag);

					bool transparent = !(tag & 0x0001);
					tag = tag >> 1;

					if(transparent)
					{
						j += tag;
						continue;
					}

					for(; tag && j < width; ++j, --tag)
					{
						uint16_t color;
						fread(&color, sizeof(color), 1, file);

						if(!_565)
						{
							color = ((color & 0xFFE0) << 1) | (color & 0x001F);
						}

						image.setPixel(j, i, ChromaKey(color));
					}
				}
			}

			fsetpos(file, &current_pos);
		}
		else
		{
			fpos_t current_pos;
			fgetpos(file, &current_pos);

			offset = byte_swap(offset);
			fseek(file, offset, SEEK_SET);

			for(int i = 0; i < height; ++i)
			{
				for(int j = 0; j < width; ++j)
				{
					uint16_t color;
					fread(&color, sizeof(color), 1, file);

					if(!color)
					{
						continue;
					}

					if(!_565)
					{
						color = ((color & 0xFFE0) << 1) | (color & 0x001F);
					}

					image.setPixel(j, i, from565(color));
				}
			}

			fsetpos(file, &current_pos);
		}

		if(m_settings.reverse_dithering)
		{
			for(uint8_t i = 0; i < m_settings.blur_iterations; ++i)
			{
				if(!blur_colors(image))
				{
					break;
				}

				if(i + 1 != m_settings.blur_iterations)
				{
					progress.setValue(++prog);
					if(prog & 0x01) qApp->processEvents();
				}
			}

			prog = m_settings.import_time*i + m_settings.blur_iterations;

			for(uint8_t i = 0; i < m_settings.alpha_iterations; ++i)
			{
				if(!blur_alpha(image))
				{
					break;
				}

				if(i + 1 != m_settings.alpha_iterations)
				{
					progress.setValue(++prog);
					if(prog & 0x01) qApp->processEvents();
				}
			}

			prog = m_settings.import_time*i + m_settings.blur_iterations + m_settings.alpha_iterations;

			progress.setValue(prog);
			if(prog & 0x01) qApp->processEvents();
		}

		if(m_settings.resize)
		{
			if(m_settings.resize_linear)
			{
				image = image.scaled(image.size()*2, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
			}
			else if(m_settings.resize_bilinear)
			{
				image = image.scaled(image.size()*2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
			}
			else
			{
				image = double_image(image);
			}
		}

		auto item = new ImageView(ui->tableWidget, ui->tableWidget->rowCount()-1, BAKED_IMAGE_SLOT);
		item->setImage(image);
		ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, BAKED_IMAGE_SLOT, item);
		ui->tableWidget->resizeRowsToContents();
		ui->tableWidget->resizeColumnsToContents();
		skip = false;
	}

	if(is_creature_sprite && m_settings.eliminate_unnecessary && m_settings.reorder_sprites)
	{
		RearrangeCommand command;
		command.rollBack(ui->tableWidget);
		ui->tableWidget->resizeRowsToContents();
		ui->tableWidget->resizeColumnsToContents();
	}

	progress.setValue(import_time);

	fclose(file);

	ui->tableWidget->setCurrentCell(0, BAKED_IMAGE_SLOT);

	imported = true;
	updateTitleBar();
}


void SpriteBuilder::importC1()
{
const static uint8_t PALETTE_DTA[] = {
	0x00,0x00,0x00, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F,
	0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x04,0x02,0x02,
	0x05,0x06,0x0A, 0x06,0x0A,0x04, 0x06,0x09,0x0C, 0x0B,0x04,0x02, 0x0A,0x06,0x09, 0x0D,0x0A,0x04,
	0x0C,0x0B,0x0C, 0x06,0x07,0x11, 0x05,0x0D,0x15, 0x06,0x0F,0x18, 0x09,0x07,0x11, 0x0B,0x0D,0x12,
	0x0B,0x0E,0x1A, 0x07,0x10,0x07, 0x07,0x10,0x0A, 0x0D,0x12,0x06, 0x0D,0x12,0x0B, 0x0F,0x18,0x06,
	0x0F,0x18,0x0A, 0x06,0x10,0x17, 0x07,0x10,0x19, 0x0D,0x11,0x14, 0x0B,0x13,0x1A, 0x0E,0x18,0x13,
	0x0F,0x18,0x1C, 0x12,0x06,0x02, 0x12,0x07,0x09, 0x14,0x0B,0x04, 0x12,0x0D,0x0B, 0x1A,0x06,0x03,
	0x1B,0x07,0x09, 0x1B,0x0C,0x04, 0x1A,0x0D,0x09, 0x12,0x0E,0x12, 0x12,0x0E,0x1A, 0x1A,0x0D,0x12,
	0x1D,0x0D,0x1A, 0x14,0x12,0x05, 0x14,0x12,0x0C, 0x14,0x19,0x06, 0x13,0x1A,0x0B, 0x1C,0x12,0x05,
	0x1B,0x13,0x0B, 0x1C,0x19,0x05, 0x1D,0x19,0x0C, 0x13,0x13,0x13, 0x13,0x15,0x1B, 0x15,0x19,0x14,
	0x15,0x19,0x1C, 0x1A,0x15,0x13, 0x1A,0x16,0x1A, 0x1C,0x1A,0x14, 0x1B,0x1B,0x1B, 0x0C,0x0F,0x21,
	0x0E,0x17,0x24, 0x10,0x0F,0x21, 0x13,0x16,0x23, 0x12,0x16,0x2C, 0x14,0x1A,0x23, 0x12,0x1B,0x2B,
	0x19,0x16,0x22, 0x19,0x17,0x2B, 0x1B,0x1C,0x23, 0x1B,0x1D,0x2A, 0x13,0x17,0x31, 0x14,0x1D,0x32,
	0x17,0x1C,0x3B, 0x1A,0x1E,0x33, 0x19,0x1E,0x3D, 0x1A,0x23,0x0D, 0x17,0x21,0x13, 0x17,0x20,0x1A,
	0x1B,0x23,0x13, 0x1D,0x22,0x1C, 0x1E,0x29,0x13, 0x1E,0x29,0x1A, 0x16,0x20,0x23, 0x17,0x20,0x2E,
	0x1C,0x21,0x25, 0x1D,0x22,0x2B, 0x1F,0x29,0x23, 0x1E,0x29,0x2C, 0x16,0x21,0x33, 0x16,0x24,0x39,
	0x16,0x29,0x3C, 0x1C,0x22,0x33, 0x1D,0x22,0x3F, 0x1E,0x28,0x36, 0x1C,0x29,0x3B, 0x23,0x06,0x04,
	0x24,0x07,0x09, 0x22,0x0D,0x04, 0x23,0x0D,0x0A, 0x2B,0x06,0x04, 0x2B,0x07,0x08, 0x2A,0x0C,0x04,
	0x2B,0x0C,0x0A, 0x26,0x0D,0x12, 0x23,0x13,0x05, 0x23,0x14,0x0A, 0x24,0x1A,0x05, 0x24,0x1A,0x0C,
	0x2B,0x14,0x05, 0x2A,0x15,0x0A, 0x2C,0x1A,0x05, 0x2B,0x1B,0x0B, 0x22,0x15,0x12, 0x22,0x16,0x1B,
	0x23,0x1B,0x13, 0x22,0x1D,0x1B, 0x2B,0x14,0x12, 0x2C,0x15,0x19, 0x2A,0x1D,0x12, 0x2B,0x1D,0x1A,
	0x34,0x0B,0x07, 0x35,0x0D,0x12, 0x32,0x15,0x05, 0x32,0x15,0x0A, 0x33,0x1A,0x05, 0x33,0x1C,0x0B,
	0x3A,0x14,0x05, 0x3A,0x14,0x0B, 0x3A,0x1D,0x05, 0x3A,0x1D,0x0A, 0x33,0x14,0x12, 0x33,0x15,0x19,
	0x33,0x1D,0x12, 0x32,0x1D,0x1A, 0x3A,0x14,0x14, 0x3B,0x16,0x18, 0x3C,0x1C,0x12, 0x3B,0x1C,0x1C,
	0x24,0x0F,0x21, 0x23,0x14,0x21, 0x21,0x1E,0x24, 0x21,0x1E,0x2A, 0x2A,0x1E,0x22, 0x29,0x1F,0x29,
	0x20,0x1F,0x31, 0x34,0x0C,0x20, 0x36,0x1C,0x22, 0x3B,0x1D,0x33, 0x29,0x22,0x0B, 0x25,0x21,0x14,
	0x24,0x22,0x1C, 0x22,0x2B,0x14, 0x23,0x2B,0x1B, 0x2C,0x22,0x14, 0x2B,0x23,0x1B, 0x2D,0x29,0x14,
	0x2D,0x2A,0x1C, 0x27,0x31,0x0F, 0x29,0x34,0x17, 0x34,0x22,0x06, 0x34,0x22,0x0C, 0x35,0x2A,0x05,
	0x34,0x2A,0x0B, 0x3C,0x23,0x05, 0x3B,0x23,0x0B, 0x3D,0x2B,0x05, 0x3D,0x2B,0x0C, 0x33,0x23,0x13,
	0x32,0x25,0x1A, 0x34,0x2A,0x14, 0x34,0x2A,0x1C, 0x3B,0x24,0x12, 0x3B,0x24,0x19, 0x3C,0x2B,0x13,
	0x3B,0x2C,0x1B, 0x34,0x31,0x0E, 0x3D,0x33,0x03, 0x3E,0x33,0x0C, 0x3F,0x3C,0x03, 0x3F,0x3B,0x0B,
	0x35,0x31,0x14, 0x35,0x31,0x1C, 0x32,0x3D,0x14, 0x33,0x3D,0x1B, 0x3E,0x32,0x13, 0x3D,0x33,0x1B,
	0x3E,0x3B,0x13, 0x3F,0x3A,0x1C, 0x23,0x22,0x24, 0x23,0x24,0x2B, 0x24,0x2A,0x24, 0x25,0x2A,0x2D,
	0x2A,0x24,0x23, 0x29,0x26,0x2C, 0x2C,0x2A,0x24, 0x2B,0x2A,0x2D, 0x22,0x25,0x33, 0x21,0x26,0x3E,
	0x25,0x29,0x34, 0x24,0x2A,0x3F, 0x28,0x27,0x31, 0x2B,0x2B,0x33, 0x29,0x2E,0x3D, 0x2A,0x32,0x2A,
	0x26,0x31,0x31, 0x2C,0x30,0x34, 0x2A,0x31,0x3F, 0x2C,0x3A,0x31, 0x2E,0x39,0x3A, 0x33,0x24,0x24,
	0x32,0x26,0x29, 0x33,0x2C,0x23, 0x32,0x2C,0x2C, 0x3B,0x24,0x23, 0x3B,0x24,0x29, 0x3A,0x2D,0x22,
	0x3A,0x2D,0x2A, 0x31,0x2E,0x32, 0x31,0x2F,0x38, 0x3D,0x2B,0x33, 0x35,0x32,0x24, 0x34,0x32,0x2C,
	0x33,0x3C,0x22, 0x33,0x39,0x2C, 0x3C,0x33,0x24, 0x3B,0x34,0x2B, 0x3E,0x3A,0x24, 0x3E,0x3B,0x2C,
	0x35,0x32,0x33, 0x32,0x32,0x3A, 0x35,0x39,0x33, 0x36,0x3A,0x39, 0x39,0x35,0x34, 0x38,0x34,0x38,
	0x3C,0x3A,0x34, 0x3D,0x3D,0x3B, 0x3F,0x3F,0x3F, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00,
	0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F,
	0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F, 0x3F,0x3F,0x3F };

	if(!documentPreClose())
	{
		return;
	}

	QString name = QFileDialog::getOpenFileName(this, tr("Open Creatures Sprite"), QString(), tr("Creatures Sprite (*.spr)"));

	if(name.isEmpty())
	{
		return;
	}

	FILE * file = fopen(name.toStdString().c_str(), "rb");

	if(!file)
	{
		QMessageBox mesg;
		mesg.setText(QObject::tr("Unable to open file '%1' for reading.").arg(filename));
		mesg.exec();
		importC1();
		return;
	}

	m_settings.Update(this);

	if(!m_settings.accepted)
	{
		fclose(file);
		return;
	}

	ui->tableWidget->clearContents();
	ui->tableWidget->setRowCount(0);
	autosave_timer.stop();
	imported = false;

	filename = name;
	ui->tableWidget->command_list.clear();

	uint16_t length;
	fread(&length, 2, 1, file);

	length = byte_swap(length);
	int import_time = m_settings.import_time*length;

	name = QFileInfo(filename ).fileName();
	bool is_creature_sprite = validator.exactMatch(name);
	char body_part = tolower(name.at(0).toLatin1());
	uint8_t part = getPartNumber(body_part);

	QProgressDialog progress(tr("Importing Sprite '%1'").arg(name), "Abort Import", 0, import_time, this);
	progress.setMinimumDuration(0);
	progress.show();

	PACK(struct, spr_header)
	{
		spr_header() :
			offset(0),
			width(0),
			height(0)
		{
		}

		void read(FILE * file)
		{
			fread(&offset, 4, 1, file);
			fread(&width, 2, 1, file);
			fread(&height, 2, 1, file);

			offset = byte_swap(offset);
			width = byte_swap(width);
			height = byte_swap(height);
		}

		uint32_t offset;
		uint16_t width, height;
	};
	UNPACK

	std::vector<spr_header> header(length);

	for(uint16_t i = 0; i < length; ++i)
	{
		header[i].read(file);
	}

	for(uint16_t i = 0; i < length; ++i)
	{
		int prog = i*m_settings.import_time;
		progress.setValue(prog);
		if(prog & 0x01) qApp->processEvents();

		if(progress.wasCanceled())
		{
			break;
		}

		if(is_creature_sprite
		&& m_settings.eliminate_unnecessary)
		{
			int c = i % 13;

			if(c < 8
			&& c != part
			&& c != part+4)
			{
				continue;
			}
		}

		ui->tableWidget->insertRow(ui->tableWidget->rowCount());

		if(is_creature_sprite)
		{
			if(m_settings.bilaterally_symmetrical)
			{
				if(body_part == 'b')
				{
					if(i % 10 < 4)
					{
						continue;
					}
				}
				else if(body_part == 'a')
				{
					if(i % 13 < 4)
					{
						continue;
					}
				}
			}

			if(m_settings.eliminate_reverse_blink)
			{
				if(body_part == 'a')
				{
					if(i == 22)
					{
						continue;
					}
				}
			}
		}

		QImage image(
			m_settings.resize? ((header[i].width +1) & 0xFFFE) : ((header[i].width +3) & 0xFFFE),
			m_settings.resize? ((header[i].height+1) & 0xFFFE) : ((header[i].height+3) & 0xFFFC), QImage::Format_ARGB32);

		image.fill(0);

		fseek(file, header[i].offset, SEEK_SET);

		for(int y = 0; y < header[i].height; ++y)
		{
			for(int x = 0; x < header[i].width; ++x)
			{
				uint8_t value;
				fread(&value, 1, 1, file);

				int red   = PALETTE_DTA[value*3] << 2;
				int green = PALETTE_DTA[value*3 + 1] << 2;
				int blue  = PALETTE_DTA[value*3 + 2] << 2;

				QRgb color = (red << 16) | (green << 8) | blue;

				if(color)
				{
					image.setPixel(x, y, 0xFF000000 | color);
				}
			}
		}

		if(m_settings.reverse_dithering)
		{
			for(uint8_t i = 0; i < m_settings.blur_iterations; ++i)
			{
				if(!blur_colors(image))
				{
					break;
				}

				if(i + 1 != m_settings.blur_iterations)
				{
					progress.setValue(++prog);
					if(prog & 0x01) qApp->processEvents();
				}
			}

			prog = m_settings.import_time*i + m_settings.blur_iterations;

			for(uint8_t i = 0; i < m_settings.alpha_iterations; ++i)
			{
				if(!blur_alpha(image))
				{
					break;
				}

				if(i + 1 != m_settings.alpha_iterations)
				{
					progress.setValue(++prog);
					if(prog & 0x01) qApp->processEvents();
				}
			}

			prog = m_settings.import_time*i + m_settings.blur_iterations + m_settings.alpha_iterations;

			progress.setValue(prog);
			if(prog & 0x01) qApp->processEvents();
		}

		if(m_settings.resize)
		{
			if(m_settings.resize_linear)
			{
				image = image.scaled(image.size()*2, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
			}
			else if(m_settings.resize_bilinear)
			{
				image = image.scaled(image.size()*2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
			}
			else
			{
				image = double_image(image);
			}
		}

		auto item = new ImageView(ui->tableWidget, ui->tableWidget->rowCount()-1, BAKED_IMAGE_SLOT);
		item->setImage(image);
		ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, BAKED_IMAGE_SLOT, item);
		ui->tableWidget->resizeRowsToContents();
		ui->tableWidget->resizeColumnsToContents();
	}

	if(is_creature_sprite && m_settings.eliminate_unnecessary && m_settings.reorder_sprites)
	{
		if(body_part == 'a')
		{
			std::vector<QTableWidgetItem*> images;

			for(uint8_t i = 0; i < ui->tableWidget->rowCount(); ++i)
			{
				images.push_back(ui->tableWidget->takeItem(i, 3));
			}

			while(ui->tableWidget->rowCount() < 32)
			{
				ui->tableWidget->insertRow(ui->tableWidget->rowCount());
			}

			ui->tableWidget->setItem(0, BAKED_IMAGE_SLOT, images[3]);
			ui->tableWidget->setItem(1, BAKED_IMAGE_SLOT, images[2]);
			ui->tableWidget->setItem(2, BAKED_IMAGE_SLOT, images[1]);
			ui->tableWidget->setItem(3, BAKED_IMAGE_SLOT, images[0]);

			ui->tableWidget->setItem(4, BAKED_IMAGE_SLOT, images[10]);
			ui->tableWidget->setItem(5, BAKED_IMAGE_SLOT, images[9]);
			ui->tableWidget->setItem(6, BAKED_IMAGE_SLOT, images[8]);
			ui->tableWidget->setItem(7, BAKED_IMAGE_SLOT, images[7]);

			ui->tableWidget->setItem( 9, BAKED_IMAGE_SLOT, images[4]);
			if(images.size() > 11) ui->tableWidget->setItem(13, BAKED_IMAGE_SLOT, images[11]);

			ui->tableWidget->setItem(17, BAKED_IMAGE_SLOT, images[5]);
			if(images.size() > 12) ui->tableWidget->setItem(21, BAKED_IMAGE_SLOT, images[12]);

			ui->tableWidget->setItem(25, BAKED_IMAGE_SLOT, images[6]);
			if(images.size() > 13) ui->tableWidget->setItem(29, BAKED_IMAGE_SLOT, images[13]);
		}
		else
		{
			RearrangeCommand command;
			command.rollBack(ui->tableWidget);
		}

		ui->tableWidget->resizeRowsToContents();
		ui->tableWidget->resizeColumnsToContents();
	}

	fclose(file);

	ui->tableWidget->setCurrentCell(0, BAKED_IMAGE_SLOT);

	imported = true;
	updateTitleBar();
}
#endif
