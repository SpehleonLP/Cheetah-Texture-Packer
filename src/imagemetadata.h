#ifndef IMAGEMETADATA_H
#define IMAGEMETADATA_H
#include <glm/gtc/type_precision.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <memory>

struct ImageMetadata
{
	uint32_t hash{};
	uint16_t packerId{};
	uint16_t duplicateId{};

	union
	{
        int id{};

		struct
		{
            uint16_t imageId;
            uint16_t frameId;
		};
	};

	glm::u16vec4 originalCoords{0, 0, 0, 0};

	glm::u16vec2 position{0, 0};
	glm::u16vec2 cropSize{0, 0};
	glm::u16vec2 currentSize{0, 0};

	bool rotated{};
	bool cropped{};
};

#endif // IMAGEMETADATA_H
