#include "packaccessor.h"
#include "Sprite/spritejson.h"
#include "widgets/glviewwidget.h"
#include <cstring>


PackMemo::BufferPtr::BufferPtr(BufferPtr && it)
{
	memcpy(this, &it, sizeof(*this));
	it.ownsData = false;
}

int32_t PackMemo::PackBufferView(uint8_t * ptr, uint32_t size, bool take_memory)
{
	BufferPtr buffer;
	buffer.data = ptr;
	buffer.size = size;
	buffer.ownsData = take_memory;

	int r = m_buffers.size();
	m_buffers.push_back(std::move(buffer));

	return r;
}

void PackMemo::InterlaceBuffers(std::unique_ptr<uint32_t[]> array, uint32_t size)
{
	BufferViewMemo view;
	view.accessors = std::move(array);
	view.length	   = size;

	m_memo.push_back(std::move(view));
}

int32_t PackMemo::PackIndices(uint16_t * ptr, uint32_t size, bool take_memory)
{
	BufferPtr buffer;
	buffer.data = (uint8_t*)ptr;
	buffer.size = size * 2;
	buffer.ownsData = take_memory;
	buffer.targetType = TargetType::ElementArrayBuffer;

	int r = m_buffers.size();
	m_buffers.push_back(std::move(buffer));

	AccessorMemo accessor;

	accessor.bufferId		= r;
	accessor.type			= Type::Scalar;
	accessor.componentType  = ComponentType::UnsignedShort;
	accessor.count			= size;

	r = m_accessors.size();
	m_accessors.push_back(std::move(accessor));

	return r;

}

int32_t PackMemo::PackAccessor(const void * ptr, uint32_t size, ComponentType component_type, Type type, bool normalize, bool take_memory)
{
	BufferPtr buffer;
	buffer.data = (uint8_t*)ptr;
	buffer.size = size * 2;
	buffer.ownsData = take_memory;
	buffer.targetType = TargetType::ElementArrayBuffer;

	int r = m_buffers.size();
	m_buffers.push_back(std::move(buffer));

	AccessorMemo accessor;

	accessor.bufferId		= r;
	accessor.type			= type;
	accessor.normalize		= normalize;
	accessor.componentType  = component_type;
	accessor.count			= size;

	r = m_accessors.size();
	m_accessors.push_back(std::move(accessor));

	return r;

}

void PackMemo::PackDocument(Sprites::Document & doc)
{
	doc.buffers.resize(1);

	//push non-interleaved
	for(uint32_t i = 0; i < m_buffers.size(); ++i)
	{
		auto & buf = m_buffers[i];

		if(buf.interleaved)
			continue;

		fx::gltf::BufferView view;

		view.buffer		= 0;
		view.byteOffset = doc.buffers[0].data.size();
		view.byteOffset += ((view.byteOffset % 4) != 0) * (4 - (view.byteOffset % 4));

		view.byteLength = buf.size;
		view.byteStride = 0;

		view.target     = buf.targetType;


		doc.bufferViews.push_back(view);

		doc.buffers[0].data.resize(view.byteOffset + view.byteLength);
		auto begin = &doc.buffers[0].data[view.byteOffset];

		memcpy(begin, buf.data, view.byteLength);
	}

//push interleaved
	for(uint32_t i = 0; i < m_memo.size(); ++i)
	{
		fx::gltf::BufferView view;

		const int view_id = doc.bufferViews.size();

		view.byteOffset = doc.buffers[0].data.size();
		view.byteOffset += ((view.byteOffset % 4) != 0) * (4 - (view.byteOffset % 4));
		view.target     =  TargetType::ArrayBuffer;

		int total_items = -1;

		for(uint32_t j = 0; j < m_memo[i].length; ++j)
		{
			auto & acc = m_accessors[m_memo[i].accessors[j]];
			auto & buf = m_buffers[acc.bufferId];

			assert(buf.bufferViewId == -1);

			buf.bufferViewId = view_id;
			acc.byteOffset   = view.byteStride;

			view.byteStride += buf.stride;
			view.byteLength += buf.stride * buf.size;

			if(total_items < 0)
				total_items = buf.size;
			else
			{
				assert(total_items == (int)buf.size);
			}
		}

		doc.bufferViews.push_back(view);

		doc.buffers[0].data.resize(view.byteOffset + view.byteLength);
		auto begin = &doc.buffers[0].data[view.byteOffset];

//copy into buffer.
		for(int k = 0; k < total_items; ++k)
		{
			for(uint32_t j = 0; j < m_memo[i].length; ++j)
			{
				auto & acc = m_accessors[m_memo[i].accessors[j]];
				auto & buf = m_buffers[acc.bufferId];

				memcpy(begin, buf.data + k * buf.stride, buf.stride);
				begin += buf.stride;
			}
		}
	}

//create accessors
	for(uint32_t i = 0; i < m_accessors.size(); ++i)
	{
		auto & acc = m_accessors[i];
		auto & buf = m_buffers[acc.bufferId];

		fx::gltf::Accessor a;

		a.bufferView	= buf.bufferViewId;
		a.byteOffset	= acc.byteOffset;
		a.count			= acc.count;
		a.normalized	= acc.normalize;
		a.componentType = acc.componentType;
		a.type			= acc.type;

		GetMinMax(m_accessors[i], a.min, a.max);
		doc.accessors.push_back(std::move(a));
	}

	doc.buffers[0].byteLength = doc.buffers[0].data.size();
}

template<typename T, int elements>
void GetMinMax2(void const* _array, uint32_t length, uint32_t stride, std::vector<float> * min, std::vector<float> * max)
{
	min->resize(elements, 0);
	max->resize(elements, 0);

	if(length > 0)
	{
		for(uint32_t i = 0; i < min->size(); ++i)
		{
			(*min)[i] = (*max)[i] = ((T*)_array)[i];
		}
	}

	for(uint32_t i = 1; i < length; ++i)
	{
		auto item = (T*)((uint8_t*)_array + stride * i);

		for(uint32_t j = 0; j < min->size(); ++j)
		{
			(*min)[j] = std::min<float>((*min)[j], item[j]);
			(*max)[j] = std::max<float>((*max)[j], item[j]);
		}
	}
}

template<int elements, typename... Args>
void MinMaxTrampoline2(fx::gltf::Accessor::ComponentType ComponentType, Args&&... args)
{
	switch((uint32_t)ComponentType)
	{
	case GL_BYTE:
		return GetMinMax2<int8_t, elements>(std::move(args)...);
	case GL_UNSIGNED_BYTE:
		return GetMinMax2<uint8_t, elements>(std::move(args)...);
	case GL_SHORT:
		return GetMinMax2<int16_t, elements>(std::move(args)...);
	case GL_UNSIGNED_SHORT:
		return GetMinMax2<uint16_t, elements>(std::move(args)...);
	case GL_INT:
		return GetMinMax2<int32_t, elements>(std::move(args)...);
	case GL_UNSIGNED_INT:
		return GetMinMax2<uint32_t, elements>(std::move(args)...);
	case GL_FLOAT:
		return GetMinMax2<float, elements>(std::move(args)...);
	case GL_DOUBLE:
		return GetMinMax2<double, elements>(std::move(args)...);
	}
}

template<typename... Args>
void MinMaxTrampoline1(fx::gltf::Accessor::Type Type, Args&&... args)
{
typedef fx::gltf::Accessor::Type k;

	switch(Type)
	{
	case k::None:
		return MinMaxTrampoline2<1>(std::move(args)...);
	case k::Scalar:
		return MinMaxTrampoline2<1>(std::move(args)...);
	case k::Vec2:
		return MinMaxTrampoline2<2>(std::move(args)...);
	case k::Vec3:
		return MinMaxTrampoline2<3>(std::move(args)...);
	case k::Vec4:
		return MinMaxTrampoline2<4>(std::move(args)...);
	case k::Mat2:
		return MinMaxTrampoline2<4>(std::move(args)...);
	case k::Mat3:
		return MinMaxTrampoline2<9>(std::move(args)...);
	case k::Mat4:
		return MinMaxTrampoline2<16>(std::move(args)...);
	}
}

void PackMemo::GetMinMax(AccessorMemo const& memo, std::vector<float> & min, std::vector<float> & max)
{
	auto & buf = m_buffers[memo.bufferId];

	return MinMaxTrampoline1(
		memo.type,
		memo.componentType,
		buf.data,
		buf.size,
		buf.stride,
		&min,
		&max);
}
