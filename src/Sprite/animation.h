#ifndef ANIMATION_H
#define ANIMATION_H
#include "Support/shared_array.hpp"
#include "Support/counted_string.h"
#include <array>
#include <atomic>

namespace Sprites
{
struct Animation;
}

class QString;
class Object;

struct Animation
{
	enum class Field
	{
		Base,
		Frames,
		LoopStart,
		LoopEnd,
		FPS,
		Total
	};

	static std::array<const char *, (int)Field::Total> FieldLabels;

	Animation() = default;
	Animation(Animation const& it) { *this = it; }

	bool SetField(Field, QString const& data, Object * parent, QString * what);

	counted_string         name;
	shared_array<uint16_t> frames;
	uint32_t				base{};
	float                  fps{20};
	uint16_t              loop_start{0};
	uint16_t              loop_end{(uint16_t)-1};

	Sprites::Animation PackDocument() const;

	Animation& operator=(Animation const& it)
	{
		name	   = it.name;
		frames	   = it.frames;
		fps		   = it.fps;
		base	   = it.base;
		loop_start = it.loop_start;
		loop_end   = it.loop_end;

		return *this;
	}

	void AddRef() const { ++m_refCount; }
	void Release() { if(--m_refCount == 0) delete this; }

private:
	mutable std::atomic<int> m_refCount{1};
};

#endif // ANIMATION_H
