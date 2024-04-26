#ifndef ANIMATION_H
#define ANIMATION_H
#include "Support/shared_array.hpp"
#include "Support/counted_string.h"
#include <atomic>

namespace Sprites
{
struct Animation;
}

struct Animation
{
	Animation() = default;
	Animation(Animation const& it) { *this = it; }

	counted_string              name;
	shared_array<uint16_t> frames;
	float                       fps{20};
	uint16_t              loop_start{0};
	uint16_t              loop_end{0};

	Sprites::Animation PackDocument() const;

	Animation& operator=(Animation const& it)
	{
		name   = it.name;
		frames = it.frames;
		fps    = it.fps;

		return *this;
	}

	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) delete this; }

private:
	mutable std::atomic<int> m_refCount{1};
};

#endif // ANIMATION_H
