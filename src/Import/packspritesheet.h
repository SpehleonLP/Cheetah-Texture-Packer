#ifndef PACKSPRITESHEET_H
#define PACKSPRITESHEET_H
#include <glm/vec2.hpp>
#include <glm/gtc/type_precision.hpp>
#include "Support/shared_array.hpp"
#include <vector>

class GLViewWidget;

struct PackSpriteSheet
{
	PackSpriteSheet(shared_array<glm::u16vec2> sizes);

	glm::u16vec2 size{0, 0};
	glm::u8vec4  border_color{0, 0, 0, 0};

	shared_array<glm::u16vec2> positions;
	shared_array<glm::u16vec2> sizes;

	shared_array<glm::i16vec4> BuildSprites();
	uint32_t UploadData(GLViewWidget *gl, void ** sprites, uint32_t internal_format, uint32_t format, uint32_t type, float compression_ratio = 0.f);

private:
	struct SheetMemo
	{
		uint32_t perimeter() { return (size.x + size.y)*2; }

		glm::u16vec2 size;
		std::vector<int> row_heights;
	};

	static uint32_t SumArea(glm::u16vec2 const* sprites, uint32_t no_sprites, bool padding);
	static SheetMemo CreateMemo(glm::u16vec2 const* sprites, uint32_t first_cut, uint32_t no_sprites);


};

#endif // PACKSPRITESHEET_H
