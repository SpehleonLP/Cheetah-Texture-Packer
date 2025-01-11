#include "animation.h"
#include "spritejson.h"
#include "Sprite/object.h"
#include "widgets/spritemodel.h"
#include <QString>

std::array<const char *, (int)Animation::Field::Total> Animation::FieldLabels =
{
	"Base",
	"Frames",
	"Loop Start",
	"Loop End",
	"FPS",
};

Sprites::Animation Animation::PackDocument() const
{
	Sprites::Animation r;

	r.name = name.toStdString();
	r.frames.insert(r.frames.end(), frames.begin(), frames.end());
	r.fps = fps;
	r.base = base;
	r.loop_start = loop_start;
	r.loop_end   = loop_end;

	return r;
}

template<typename T>
bool SetValue(T & value, QString const& str)
{
	bool okay;
	float v = str.toFloat(&okay);
	if(!okay) return false;
	value = v;
	return true;
}

bool Animation::SetField(Field field, QString const& data, Object * parent, QString * what)
{
	switch(field)
	{

	case Field::Base:
	{
		uint32_t base;
		if(!SetValue(base, data))
			return false;

		if(!parent->CheckIfValid(frames, base))
		{
			if(what) *what = "frame id greater than total frames in sprite sheet";
			return false;
		}

		this->base = base;
		return true;
	}
	case Field::Frames:
	{
		shared_array<uint16_t> frames;
		if(!SpriteModel::VectorFromString(frames, data))
			return false;

		if(!parent->CheckIfValid(frames, base))
		{
			if(what) *what = "frame id greater than total frames in sprite sheet";
			return false;
		}

		this->frames = frames;
		return true;
	}
	case Field::LoopStart:
		return SetValue(loop_start, data);
	case Field::LoopEnd:
		return SetValue(loop_end, data);
	case Field::FPS:
		return SetValue(fps, data);
	default:
		return false;
	}
}
