#include "unpackmemo.h"
#include "Sprite/imagemanager.h"
#include "Support/getuniquecountedarray.h"
#include "Sprite/imagetexturecoordinates.h"
#include <cstring>
#include <limits>
#include <type_traits>

UnpackMemo::UnpackMemo(std::string const& documentFilePath) : documentFilePath(documentFilePath) {}
UnpackMemo::~UnpackMemo() {}

counted_ptr<Image> UnpackMemo::UnpackImage(ImageManager * manager, UnpackMemo & memo, Sprites::Document const& doc, fx::gltf::Material::Texture const& _texture)
{
	if((uint32_t)_texture.index >= doc.textures.size())
		return {};

	auto & texture = doc.textures[_texture.index];

	if((uint32_t)texture.source >= doc.images.size())
		throw std::out_of_range("texture.source");

	if((uint32_t)texture.texCoords >= doc.texCoords.size())
		throw std::out_of_range("texture.texCoords");

	ImageKey key(memo.documentFilePath, texture.source);

	return manager->GetImage(memo, doc, texture.source, texture.texCoords);
}


counted_ptr<ImageTextureCoordinates> UnpackMemo::GetTexCoords(Sprites::Document const& doc, int i)
{
	if(m_texCoords.size() != doc.texCoords.size())
		m_texCoords.resize(doc.texCoords.size());

	if((uint32_t)i < m_texCoords.size() && !m_texCoords[i].empty())
		return m_texCoords[i];

	return (m_texCoords[i] = ImageTextureCoordinates::Factory(doc, i, *this));
}

// Procedural replacement for the old fx::gltf::stdAccessor wrapper. Reads one
// gltf accessor element-by-element, converting each stored component to the
// requested type T (normalizing integer->float only, matching the old reader),
// and supports sparse accessors. This is the only place that needed that logic.
namespace
{
typedef fx::gltf::Accessor::ComponentType gltf_ComponentType;

// read a single scalar stored as `Src` and convert it to `T`.
template<typename Src, typename T>
T ConvertScalar(uint8_t const* src, bool normalize)
{
	Src v;
	std::memcpy(&v, src, sizeof(Src));

//only normalize when going integer -> float, like the old accessor did.
	if(normalize
	&& std::is_floating_point<T>::value
	&& std::is_integral<Src>::value)
	{
		T t = v / (T) std::numeric_limits<Src>::max();
		if(std::numeric_limits<Src>::is_signed)
			t = std::max<T>(-1, t);
		return t;
	}

	return (T) v;
}

// read a single scalar of run-time component type `ct` and convert it to `T`.
template<typename T>
T ReadScalar(uint8_t const* src, gltf_ComponentType ct, bool normalize)
{
	switch(ct)
	{
	case gltf_ComponentType::Byte:          return ConvertScalar< int8_t , T>(src, normalize);
	case gltf_ComponentType::UnsignedByte:  return ConvertScalar<uint8_t , T>(src, normalize);
	case gltf_ComponentType::Short:         return ConvertScalar< int16_t, T>(src, normalize);
	case gltf_ComponentType::UnsignedShort: return ConvertScalar<uint16_t, T>(src, normalize);
	case gltf_ComponentType::Int:           return ConvertScalar< int32_t, T>(src, normalize);
	case gltf_ComponentType::UnsignedInt:   return ConvertScalar<uint32_t, T>(src, normalize);
	case gltf_ComponentType::Float:         return ConvertScalar<float   , T>(src, normalize);
	default: throw std::runtime_error("unrecognized gltf component type id.");
	}
}

// resolve a (bufferView, byteOffset) pair to a raw pointer + element stride.
struct RawView { uint8_t const* base; uint32_t stride; };

RawView ResolveBufferView(Sprites::Document const& doc, uint32_t bufferView, uint32_t byteOffset, uint32_t elementSize)
{
	if(bufferView >= doc.bufferViews.size())
		throw std::out_of_range("gltf accessor references an invalid buffer view");

	auto const& view = doc.bufferViews[bufferView];

	if(!(0 <= view.buffer && (size_t) view.buffer < doc.buffers.size()))
		throw std::out_of_range("gltf buffer view references an invalid buffer");

	auto const& buffer = doc.buffers[view.buffer];

	if(buffer.data.empty())
		throw std::logic_error("gltf buffer was not loaded");

	uint32_t stride = view.byteStride != 0 ? view.byteStride : elementSize;

	if(buffer.data.size() < view.byteOffset + byteOffset + elementSize)
		throw std::out_of_range("gltf buffer view is too short for accessor");

	return { &buffer.data[view.byteOffset + byteOffset], stride };
}
}

template<int n, typename T, glm::qualifier Q>
immutable_array<glm::vec<n, T, Q>>  LoadAccessorArray(Sprites::Document const& doc, uint32_t i, glm::vec<n, T, Q>)
{
	typedef glm::vec<n, T, Q> Vec;

	if(i >= doc.accessors.size())
		throw std::out_of_range("requested an accessor which doesn't exist.");

	auto const& accessor = doc.accessors[i];

	if(accessor.componentType == gltf_ComponentType::None)
		throw std::runtime_error("gltf accessor array is invalid, (component type null)");

	if((int) accessor.type != n)
		throw std::runtime_error("gltf accessor type does not match requested vector size");

	uint32_t componentSize = fx::gltf::Accessor::GetComponentSizeInBytes(accessor.componentType);
	uint32_t elementSize   = componentSize * n;

	auto readElement = [&](uint8_t const* src)
	{
		Vec v;
		for(int c = 0; c < n; ++c)
			v[c] = ReadScalar<T>(src + c * componentSize, accessor.componentType, accessor.normalized);
		return v;
	};

	shared_array<Vec> r(accessor.count);

	if(accessor.sparse.count > 0)
	{
//sparse: base is zero, then specific indices are overridden (matches old behavior).
		for(uint32_t e = 0; e < accessor.count; ++e)
			r[e] = Vec(0);

		auto const& sparse = accessor.sparse;
		uint32_t indexSize = fx::gltf::Accessor::GetComponentSizeInBytes(sparse.indices.componentType);

		RawView indices = ResolveBufferView(doc, sparse.indices.bufferView, sparse.indices.byteOffset, indexSize);
		RawView values  = ResolveBufferView(doc, sparse.values.bufferView,  sparse.values.byteOffset,  elementSize);

		for(uint32_t s = 0; s < (uint32_t) sparse.count; ++s)
		{
			uint32_t target = ReadScalar<uint32_t>(indices.base + s * indices.stride, sparse.indices.componentType, false);
			if(target < accessor.count)
				r[target] = readElement(values.base + s * values.stride);
		}
	}
	else
	{
		RawView view = ResolveBufferView(doc, accessor.bufferView, accessor.byteOffset, elementSize);

		for(uint32_t e = 0; e < accessor.count; ++e)
			r[e] = readElement(view.base + e * view.stride);
	}

	return r;
}

template<typename T>
immutable_array<T> UnpackMemo::GetAccessor(Sprites::Document const& doc, int i, std::vector<immutable_array<T>> UnpackMemo::*array)
{
	if(i < 0) return {};

	if((this->*array).size() != doc.accessors.size())
		(this->*array).resize(doc.accessors.size());

	if((uint32_t)i < (this->*array).size() && !(this->*array)[i].empty())
		return (this->*array)[i];

	return ((this->*array)[i] = MakeUnique(LoadAccessorArray(doc, i, T())));
}

immutable_array<glm::i16vec4> UnpackMemo::GetAccessor_i16vec4(Sprites::Document const& doc, int i)
{
	return GetAccessor<glm::i16vec4>(doc, i, &UnpackMemo::m_i16vec4Accessors);
}

immutable_array<glm::u16vec4> UnpackMemo::GetAccessor_u16vec4(Sprites::Document const& doc, int i)
{
	return GetAccessor<glm::u16vec4>(doc, i, &UnpackMemo::m_u16vec4Accessors);
}

immutable_array<glm::i16vec2> UnpackMemo::GetAccessor_i16vec2(Sprites::Document const& doc, int i)
{
	return GetAccessor<glm::i16vec2>(doc, i, &UnpackMemo::m_i16vec2Accessors);
}

