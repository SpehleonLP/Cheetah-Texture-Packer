#include "packspritesheet.h"
#include <widgets/glviewwidget.h>
#include <climits>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream>


bool IsCompressed(uint32_t u)
{
	switch(u)
	{
	case GL_COMPRESSED_RED:
	case GL_COMPRESSED_RG:
	case GL_COMPRESSED_RGB:
	case GL_COMPRESSED_RGBA:
	case GL_COMPRESSED_SRGB:
	case GL_COMPRESSED_SRGB_ALPHA:
	case GL_COMPRESSED_RED_RGTC1:
	case GL_COMPRESSED_SIGNED_RED_RGTC1:
	case GL_COMPRESSED_RG_RGTC2:
	case GL_COMPRESSED_SIGNED_RG_RGTC2:
	case GL_COMPRESSED_RGBA_BPTC_UNORM:
	case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
	case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
	case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
		return true;
	}

	return false;
}

PackSpriteSheet::PackSpriteSheet(shared_array<glm::u16vec2> _sizes) :
	sizes(std::move(_sizes))
{
//	uint32_t  optimal_area = SumArea(&sizes[0], sizes.size(), true);
	SheetMemo best         = CreateMemo(&sizes[0], 1, sizes.size());
//	float     max         = optimal_area / (float) (best.size.x * best.size.y);


	for(uint32_t i = 2; i <= sizes.size(); ++i)
	{
		SheetMemo eax = CreateMemo(&sizes[0], i, sizes.size());
		//float     value = optimal_area / (float) (eax.size.x * eax.size.y);

		if(eax.perimeter() < best.perimeter())
			best = eax;
	}


	positions = shared_array<glm::u16vec2>(sizes.size());
	size      = best.size;

	glm::u16vec2 row  {4, 4};

	for(uint32_t i = 0, j = 0; i < sizes.size(); ++i)
	{
		glm::u16vec2 rounded = (sizes[i] + glm::u16vec2(7, 7)) & glm::u16vec2(0xFFFC, 0xFFFC);

		if(row.x + rounded.x > best.size.x)
		{
			row.x = 4;
			row.y += best.row_heights[j++];
		}

		positions[i] = row;
		row.x += rounded.x;
	}
}

uint32_t PackSpriteSheet::SumArea(glm::u16vec2 const* sprites, uint32_t no_sprites, bool padding)
{
	uint32_t r{};

	for(uint32_t i = 0; i < no_sprites; ++i)
	{
		if(padding) 	r += (sprites[i].x+4) * (sprites[i].y+4);
		else			r += sprites[i].x * sprites[i].y;
	}

	return r;
}

PackSpriteSheet::SheetMemo PackSpriteSheet::CreateMemo(glm::u16vec2 const* sprites, uint32_t first_cut, uint32_t no_sprites)
{
	uint16_t max_width{0};

	glm::u16vec2 row  {4, 4};
	glm::u16vec2 sheet{4, 4};

	for(uint32_t i = 0; i < first_cut; ++i)
	{
		glm::u16vec2 rounded = (sprites[i] + glm::u16vec2(7, 7)) & glm::u16vec2(0xFFFC, 0xFFFC);

		row.y = std::max<int>(rounded.y, row.y);
		row.x += rounded.x;
	}

	std::vector<int> row_start;
	row_start.push_back(row.y);

	sheet = row;

	if(first_cut < no_sprites) row.x += sprites[first_cut].x;
	max_width = row.x;

	row = glm::u16vec2(4, 4);

	for(uint32_t i = first_cut; i < no_sprites; ++i)
	{
		glm::u16vec2 rounded = (sprites[i] + glm::u16vec2(7, 7)) & glm::u16vec2(0xFFFC, 0xFFFC);

		if(row.x + rounded.x > max_width)
		{
			row_start.push_back(row.y);

			sheet.x = std::max<int>(sheet.x, row.x+4);
			sheet.y += row.y;

			row = glm::u16vec2(4, 4);
		}

		row.y  = std::max<int>(rounded.y, row.y);
		row.x += rounded.x;
	}

	sheet.x = std::max<int>(sheet.x, row.x+4);
	sheet.y += row.y;

	return { glm::u16vec2(sheet.x, sheet.y+4), std::move(row_start) };
}

shared_array<glm::i16vec4> PackSpriteSheet::BuildSprites()
{
	shared_array<glm::i16vec4> r(positions.size());

	for(uint32_t i = 0; i < positions.size(); ++i)
		r[i] = glm::u16vec4(positions[i], 0, 0);

	for(uint32_t i = 0; i < sizes.size(); ++i)
		r[i] = glm::u16vec4(r[i].x, r[i].y, r[i].x + sizes[i].x, r[i].y + sizes[i].y);

	return r;
}

uint32_t PackSpriteSheet::UploadData(GLViewWidget *gl, void ** sprites, uint32_t internal_format, uint32_t format, uint32_t type, float compression_ratio)
{
	using namespace std::chrono_literals;
	uint32_t r;

	try
	{
		_gl glGenTextures(1, &r);

		_gl glActiveTexture(GL_TEXTURE0);
		_gl glBindTexture(GL_TEXTURE_2D, r);

		_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

		_gl glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		_gl glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		

		uint32_t N = size.x*size.y;
		std::unique_ptr<glm::u8vec4[]> background_color(new glm::u8vec4[N]);

		for(uint32_t i = 0; i < N; ++i)
			background_color[i] = border_color;

		_gl glTexImage2D(GL_TEXTURE_2D,
			0,
			internal_format,
			size.x,
			size.y,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			&background_color[0][0]);

		

		bool is_compressed = IsCompressed(format);

		for(uint32_t i = 0; i < positions.size(); ++i)
		{
			glm::u16vec4 coords = glm::u16vec4(positions[i], sizes[i].x, sizes[i].y);

//			std::cerr << "AABB[" << i << "] = {" << coords.x << ", " << coords.y << ", " << coords.z  << ", " << coords.w  << "}" << std::endl;

			assert( coords.x + coords.z < size.x && coords.y + coords.w < size.y);
			assert(size.x > 0 && size.y > 0);

			if(!is_compressed)
				_gl glTexSubImage2D(GL_TEXTURE_2D,
					0,
					coords.x,
					coords.y,
					coords.z,
					coords.w,
					format,
					type,
					sprites[i]);
			else
				_gl glCompressedTexSubImage2D(GL_TEXTURE_2D,
					0,
					coords.x,
					coords.y,
					coords.z,
					coords.w,
					format,
					coords.z * coords.w * compression_ratio,
					sprites[i]);

			
		}

		_gl glBindTexture(GL_TEXTURE_2D, 0);
	}
	catch(...)
	{
		_gl glBindTexture(GL_TEXTURE_2D, 0);
		_gl glDeleteTextures(1, &r);
		r = 0;
		throw;
	}

	

	return r;
}
