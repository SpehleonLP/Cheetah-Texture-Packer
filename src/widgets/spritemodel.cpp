#include "spritemodel.h"
#include "src/Sprite/document.h"
#include "src/commandlist.h"
#include "mainwindow.h"
#include "Sprite/object.h"
#include "ui_mainwindow.h"
#include <fx/gltf.h>
#include <iostream>
#include "Sprite/image.h"
#include <QMessageBox>

struct ModelToken
{
	MaterialProperty property;
	MaterialProperty first_child;
	uint8_t      row;
	uint8_t      depth;
	uint8_t      children;
	TokType      type;
    long         offsetof;
	const char * name;
};

template<typename T>
T & Get(Material &mat, ModelToken const& detail)
{
	return *(T*)((uint8_t*)&mat + detail.offsetof);
}


#define Token3(depth, row, children, name, tok_type, offset, string)  { MaterialProperty:: name, MaterialProperty::None, row, depth, children, TokType:: tok_type, (long) ((uint8_t*)&singleton.offset - (uint8_t*)&singleton), string  }
#define Token2(depth, row, children, name, first_child)               { MaterialProperty:: name, MaterialProperty:: first_child, row, depth, children, TokType::None, 0L, #name  }
#define Token_(depth, row, children, name, tok_type, offset)          Token3(depth, row, children, name, tok_type, offset, #name)

static ModelToken * GetModel()
{
	static Material singleton;
	static ModelToken model[] =
	{
	Token2(0, 0, 0, None		, None),
	{ MaterialProperty::Root, MaterialProperty::Name, 0, 0, 11, TokType::None, 0, "Material"  },
	Token_(1, 0, 0, Name		, String,  name ),
	Token_(1, 1, 0, AlphaCutoff,  Float,   alphaCutoff),
	Token_(1, 2, 0, AlphaMode,    AlphaMode, alphaMode),
	Token_(1, 3, 0, DoubleSided,  Boolean, doubleSided),
	Token2(1, 4, 2, Normal, NormalTexture),
	Token2(1, 5, 2, Occlusion, OcclusionTexture),
	Token2(1, 6, 5, PBRMetallicRoughness, BaseColorFactor),
	Token_(1, 7, 0, EmissiveFactor,            Vec3,    emissiveFactor),
	Token_(1, 8, 0, EmissiveTexture,           Texture, image_slots[(int)Material::Tex::Emission]),
	Token2(1, 9, 6, KHR_SpecularGlossiness,     DiffuseFactor),
	Token_(1,10, 0, Unlit,                     Boolean, ext.unlit.is_empty),

	Token_(2, 0, 0, NormalTexture,             Texture, image_slots[(int)Material::Tex::Normal]),
	Token_(2, 1, 0, NormalScale,               Float,   normalTexture.scale),

	Token_(2, 0, 0, OcclusionTexture,          Texture, image_slots[(int)Material::Tex::Occlusion]),
	Token_(2, 1, 0, OcclusionStrength,         Float,   occlusionTexture.strength),

	Token_(2, 0, 0, BaseColorFactor,           Vec4,    pbrMetallicRoughness.baseColorFactor),
	Token_(2, 1, 0, BaseColorTexture,          Texture, image_slots[(int)Material::Tex::BaseColor]),
	Token_(2, 2, 0, RoughnessFactor,           Float,   pbrMetallicRoughness.roughnessFactor),
	Token_(2, 3, 0, MetallicFactor,            Float,   pbrMetallicRoughness.metallicFactor),
	Token_(2, 4, 0, MetallicRoughnessTexture,  Texture, image_slots[(int)Material::Tex::MetallicRoughness]),

	Token_(2, 0, 0, DiffuseFactor,             Vec4,    ext.pbrSpecularGlossiness.diffuseFactor),
	Token_(2, 1, 0, DiffuseTexture,            Texture, image_slots[(int)Material::Tex::Diffuse]),
	Token_(2, 2, 0, SpecularFactor,            Vec3,    ext.pbrSpecularGlossiness.specularFactor),
	Token_(2, 2, 0, GlossinessFactor,          Float,   ext.pbrSpecularGlossiness.glossinessFactor),
	Token_(2, 3, 0, SpecularGlossinessTexture, Texture, image_slots[(int)Material::Tex::SpecularGlossiness]),
	Token3(2, 4, 0, SpecularGlossinessEnabled, Boolean, ext.pbrSpecularGlossiness.is_empty, "Is Enabled"),

	{}
	};

	return model;
};

Material::Tex GetTexture(MaterialProperty prop)
{
	switch(prop)
	{
	case MaterialProperty::NormalTexture:             return Material::Tex::Normal;
	case MaterialProperty::EmissiveTexture:           return Material::Tex::Emission;
	case MaterialProperty::OcclusionTexture:          return Material::Tex::Occlusion;
	case MaterialProperty::BaseColorTexture:          return Material::Tex::BaseColor;
	case MaterialProperty::MetallicRoughnessTexture:  return Material::Tex::MetallicRoughness;
	case MaterialProperty::DiffuseTexture:            return Material::Tex::Diffuse;
	case MaterialProperty::SpecularGlossinessTexture: return Material::Tex::SpecularGlossiness;
	default:
		break;
	}

	return Material::Tex::None;
}


SpriteModel::SpriteModel(MainWindow * parent) :
	super(parent),
	window(parent)
{
//unit test model
	auto model = GetModel();

	for(int i = 0;  !i || (int)model[i].property != 0; ++i)
	{
		if((int)model[i].property != i)
		{
			BreakIfDebugging();
			throw std::logic_error("material model corrupt");
		}
	}

}

QModelIndex SpriteModel::index(const int row, const int column, const QModelIndex &parent) const
{
	Entity entity = parent.internalPointer();

	if(!hasIndex(row, column, parent))
		return QModelIndex();


	switch(entity.heirarchy)
	{
	case Heirarchy::Root:
		if(row == 0)
			entity.heirarchy = Heirarchy::ObjectList;
		else
			entity.heirarchy = Heirarchy::Textures;
		return createIndex(row, column, entity.data());

	case Heirarchy::ObjectList:
	{
		entity.object = row;
		entity.heirarchy = Heirarchy::ObjectContents;
		return createIndex(row, column, entity.data());
	}

	case Heirarchy::ObjectContents:
		switch(row)
		{
		case 0:
			entity.heirarchy = Heirarchy::Material;
			entity.property  = MaterialProperty::Root;
			break;
		case 1:
			entity.heirarchy = Heirarchy::Animation;
			break;
		case 2:
			entity.heirarchy = Heirarchy::Attachment;
			break;
		default:
			return QModelIndex();
		}

		entity.numeric   = -1;
		entity.subId     = -1;
		return createIndex(row, column, entity.data());
	case Heirarchy::Material:
	{
		auto & mod = GetModel()[(int)entity.property];

		if(mod.children == 0)
			return QModelIndex();

		assert(row < mod.children);

		entity.property = (MaterialProperty) ((int)GetModel()[(int)entity.property].first_child + row);
		return createIndex(row, column, entity.data());
	}
	case Heirarchy::Animation:
		if(entity.numeric < 0)
			entity.numeric = row;
		else if(entity.subId < 0)
			entity.subId   = row;
		else
			return QModelIndex();

		return createIndex(row, column, entity.data());

	case Heirarchy::Attachment:
		if(entity.numeric < 0)
			entity.numeric = row;

		return createIndex(row, column, entity.data());

	case Heirarchy::Textures:
	default:
		break;
	}

	return QModelIndex();
}

QModelIndex SpriteModel::parent(const QModelIndex &child) const
{
	Entity entity = child.internalPointer();

	if(!child.isValid())return QModelIndex();

	switch(entity.heirarchy)
	{
	case Heirarchy::Root:
		return QModelIndex();
	case Heirarchy::ObjectList:
        return QModelIndex();

//		entity.object    = 0;
//		entity.heirarchy = Heirarchy::Root;
//		return createIndex(0, 0, entity.data());

	case Heirarchy::ObjectContents:
		entity.heirarchy = Heirarchy::ObjectList;
		return createIndex(entity.object, 0, entity.data());

	case Heirarchy::Material:
		if(entity.property == MaterialProperty::Root)
		{
			entity.property  = MaterialProperty::None;
			entity.heirarchy = Heirarchy::ObjectContents;

			return createIndex(0, 0, entity.data());
		}

	{
		Entity tmp = entity;
		//walk backwards until we reach item 0

		int row = (int)tmp.property;
		for(; GetModel()[row].row > 0; --row) {}

		for(int i = 0; i < (int)MaterialProperty::Total; ++i)
		{
			if((int)GetModel()[i].first_child == row)
			{
				tmp.property = (MaterialProperty)i;
				return createIndex(GetModel()[(int)entity.property].row, 0, tmp.data());
			}
		}

		return QModelIndex();
	}
	case Heirarchy::Animation:
		if(entity.numeric < 0)
		{
			entity.heirarchy = Heirarchy::ObjectContents;
			return createIndex(entity.object, 0, entity.data());
		}
		else if(entity.subId >= 0)
		{
			entity.subId   = -1;
			return createIndex(entity.numeric, 0, entity.data());
		}
		else
		{
			entity.numeric = -1;
			return createIndex(0, 0, entity.data());
		}
	case Heirarchy::Attachment:
		if(entity.numeric < 0)
		{
			entity.heirarchy = Heirarchy::ObjectContents;
			return createIndex(entity.object, 0, entity.data());
		}
		else
		{
			entity.numeric = -1;
			return createIndex(0, 0, entity.data());
		}
	case Heirarchy::Textures:
		entity.object    = 0;
		entity.heirarchy = Heirarchy::Root;
		return createIndex(1, 0, entity.data());
	default:
		break;
	}

	return QModelIndex();
}

int SpriteModel::rowCount(const QModelIndex &parent) const
{
	if(!parent.isValid())
		return 2;

	Entity entity = parent.internalPointer();

	auto doc = window->document.get();

	switch(entity.heirarchy)
	{
	case Heirarchy::Root:           return 2;
	case Heirarchy::ObjectList:  	return std::min<int>(doc->objects.size()+1, 128);
	case Heirarchy::ObjectContents:
		if((uint32_t)entity.object >= doc->objects.size())
			return 0;
		return 3;
	case Heirarchy::Material: 		return GetModel()[(int)entity.property].children;
	case Heirarchy::Animation:
	{
		if(entity.subId >= 0) return 0;

		if((uint32_t)entity.object >= doc->objects.size())
			return 0;

		auto const& vec = doc->objects[entity.object]->animations;

		if(entity.numeric < 0)
			return std::min<int>(vec.size()+1, 128);
		else if((uint32_t)entity.numeric >= vec.size())
			return 0;
		else
			return (int)Animation::Field::Total;
	}
	case Heirarchy::Attachment:
	{
		if((uint32_t)entity.object >= doc->objects.size())
			return 0;

		auto const& vec = doc->objects[entity.object]->attachments;
		return (entity.numeric < 0)? std::min<int>(vec.size()+1, 128) : 0;
	}
//	case Heirarchy::Textures:       return doc->textures.size();
	default:
		break;
	}

	return 0;
}

int SpriteModel::columnCount(const QModelIndex &parent) const
{
	if(!parent.isValid())
		return 2;

	Entity entity = parent.internalPointer();

	switch(entity.heirarchy)
	{
	case Heirarchy::Root:              return 1;
	case Heirarchy::ObjectList:        return 1;
	case Heirarchy::ObjectContents:    return 1;
	case Heirarchy::Material:          return 1 + (GetModel()[(int)entity.property].children != 0);
	case Heirarchy::Animation:		   return entity.numeric < 0? 1 : 2;
	case Heirarchy::Attachment:        return 1;
	case Heirarchy::Textures:          return 1;
	default:
		break;
	}

	return 0;
}
/*
bool SpriteModel::hasChildren(const QModelIndex &parent) const
{
	if(!parent.isValid())
		return 1;

	Entity entity = parent.internalPointer();

	switch(entity.heirarchyId)
	{
	case Heirarchy::Root: return true;
	case Heirarchy::ObjectList: return true;
	case Heirarchy::ObjectContents: return true;
	case Heirarchy::Material:
		return GetModel()[(int)entity.propertyId].children;
	case Heirarchy::Animation: return 0;
	case Heirarchy::Attachment: return 0;
	case Heirarchy::Textures: return 0;
	default:
		break;
	}

	return false;
}*/

QVariant SpriteModel::data(const QModelIndex &index, int role) const
{
	Entity entity = index.internalPointer();
	auto doc = window->document.get();

	if (role != Qt::DisplayRole
	|| !index.isValid())
        return QVariant();

	switch(entity.heirarchy)
	{
	case Heirarchy::Root: return "Root";
	case Heirarchy::ObjectList: return "Object List";
	case Heirarchy::ObjectContents:
	{
		if(index.column() > 0)
			return QVariant();

		if((uint32_t)index.row() >= doc->objects.size())
			return "Add Object...";

		return doc->objects[index.row()]->name.c_str();
	}
	case Heirarchy::Material:
	{
		if(index.column() == 0)
			return GetModel()[(int)entity.property].name;

		if((uint32_t)entity.object >= doc->objects.size())
			return false;

		auto & material =  *doc->objects[entity.object]->material.get();
		auto       detail   = GetModel()[(int)entity.property];

		switch(detail.type)
		{
		case TokType::Boolean:
			if(entity.property == MaterialProperty::Unlit)
				return material.ext.unlit.is_empty? "false" : "true";

			return Get<bool>(material, detail)? "true" : "false";
		case TokType::AlphaMode:
		{
			const char * mode[] = { "Opaque", "Mask", "Blend" };
			return mode[(int)Get<fx::gltf::Material::AlphaMode>(material, detail)];
		}
		case TokType::Float:
			return QString::number(Get<float>(material, detail));
		case TokType::Vec3:
			return StringFromArray<3>(Get<std::array<float, 3>>(material, detail));
		case TokType::Vec4:
			return StringFromArray<4>(Get<std::array<float, 4>>(material, detail));
		case TokType::String:
			return Get<std::string>(material, detail).c_str();
		case TokType::Texture:
		{
			auto ptr = Get< counted_ptr<Image> >(material, detail);

			if(ptr.empty())
				return "";

			auto filename = ptr->getFilename();
			return QString::fromStdString(std::string(filename));
		}
		default:
			break;
		}
	} break;
	case Heirarchy::Animation:
	{

		if((uint32_t)entity.object >= doc->objects.size())
			return QVariant();

		auto const& animations = doc->objects[entity.object]->animations;
		if(entity.numeric < 0)
			return "Animation List";
		else if((uint32_t)entity.numeric >= animations.size())
			return "Add Animation...";
		else
		{
			auto anim = animations[entity.numeric].get();

			if(entity.subId < 0)
				return anim->name.c_str();

			if(index.column() == 0 && (uint32_t)entity.subId < Animation::FieldLabels.size())
				return Animation::FieldLabels[entity.subId];

			switch((Animation::Field)entity.subId)
			{
			case Animation::Field::Base:
				return QString::number(anim->base);
			case Animation::Field::Frames:
				return StringFromVector(anim->frames);
			case Animation::Field::LoopStart:
				return QString::number(anim->loop_start);
			case Animation::Field::LoopEnd:
				return QString::number(anim->loop_end);
			case Animation::Field::FPS:
				return QString::number(anim->fps);
			default:
				break;
			}
		}
	} return QVariant();
	case Heirarchy::Attachment:
	{
		if((uint32_t)entity.object >= doc->objects.size())
			return QVariant();

		auto const& attachments = doc->objects[entity.object]->attachments;
		if(entity.numeric < 0)
			return "Attachment List";
		else if((uint32_t)entity.numeric >= attachments.size())
			return "Add Attachments...";
		else
			return attachments[entity.numeric].name.c_str();
	} return QVariant();
	case Heirarchy::Textures:
	{
		return "Texture List";
	}
	default:
		break;
	}

    return QVariant();
}

//	bool setHeaderData(int, Qt::Orientation, QVariant const&, int);
bool SpriteModel::setData(QModelIndex const& index,  QVariant const& variant, int role)
{
	bool okay{};
	union
	{
		int64_t Int;
		double Dbl;
	};

	Entity entity = index.internalPointer();
	Document * doc = window->document.get();

	if(!index.isValid()
	|| role != Qt::EditRole)
		return false;

	QString value = variant.value<QString>();

	switch(entity.heirarchy)
	{
	case Heirarchy::ObjectContents:
		doc->addCommand<ObjectCommand>(entity.object, value.toStdString());
		return true;
	case Heirarchy::Material:
		if(index.column() == 0)
			return false;

		return UpdateMaterial(entity, value);
	case Heirarchy::Animation:
	{
		if((uint32_t)entity.object >= doc->objects.size())
			return false;

		Animation anim;

		auto obj = doc->objects[entity.object].get();

		if((uint32_t)entity.numeric >= obj->animations.size())
		{
			anim.name = counted_string::MakeUnique(value.toStdString());
			if(anim.name.empty()) return false;
		}
		else
		{
			anim = *obj->animations[entity.numeric];

			std::vector<uint16_t> vec;

			if(entity.subId < 0)
				anim.name = counted_string::MakeUnique(value.toStdString());

			QString what;
			if(!anim.SetField((Animation::Field)entity.subId, value, doc->objects[entity.object].get(), &what))
			{
				if(what.isEmpty() == false)
					QMessageBox::warning(window, "Cheetah Animation", what);

				return false;
			}
		}

		if(anim.name.empty())
			doc->addCommand<AnimationCommand>(entity.object, entity.numeric, nullptr);
		else
			doc->addCommand<AnimationCommand>(entity.object, entity.numeric, &anim);

		return true;
	}
	case Heirarchy::Attachment:
	{
		if((uint32_t)entity.object >= doc->objects.size())
			return false;

		doc->addCommand<AttachmentCommand>(entity.object, entity.numeric, value.toStdString());
		return true;
	}
	case Heirarchy::Textures: return false;	// */
	default:
		break;

	}


	return false;
}

bool SpriteModel::UpdateMaterial(Entity entity, const QString & value)
{
typedef fx::gltf::Material::AlphaMode AlphaMode;

	auto doc = window->document.get();

	if((uint32_t)entity.object >= doc->objects.size())
		return false;

	auto & material = *doc->objects[entity.object]->material;
	auto   detail   = GetModel()[(int)entity.property];

	switch(detail.type)
	{
	case TokType::Boolean:
		doc->addCommand<UpdateMaterialCommand<bool>>(entity.object, detail.offsetof, !Get<bool>(material, detail));
		return true;
	case TokType::AlphaMode:
	{
		AlphaMode mode = Get<AlphaMode>(material, detail);
		mode = (AlphaMode)(((int)mode + 1) % 3);

		doc->addCommand<UpdateMaterialCommand<AlphaMode>>(entity.object, detail.offsetof, mode);
		return true;
	}
	case TokType::Float:
	{
		bool okay{false};
		float flt = value.toFloat(&okay);
		if(okay)
		{
			doc->addCommand<UpdateMaterialCommand<float>>(entity.object, detail.offsetof, flt);
		}

		return okay;
	}
	case TokType::Vec3:
	{
		std::array<float, 3> tmp;
		if(ArrayFromString<3>(tmp, value))
		{
			doc->addCommand<UpdateMaterialCommand< std::array<float, 3> > >(entity.object, detail.offsetof, std::move(tmp));
			return true;
		}

		return false;
	}
	case TokType::Vec4:
	{
		std::array<float, 4> tmp;
		if(ArrayFromString<4>(tmp, value))
		{
			doc->addCommand<UpdateMaterialCommand< std::array<float, 4> > >(entity.object, detail.offsetof, std::move(tmp));
			return true;
		}

		return false;
	}
	case TokType::String:
	{
		doc->addCommand<UpdateMaterialCommand<std::string> >(entity.object, detail.offsetof, value.toStdString());
	} return true;
	case TokType::Texture:
		return false;
	default:
		break;
	}

	return false;
}

bool IsAnimation(Heirarchy heirarchy, MaterialProperty property)
{
	switch(heirarchy)
	{
	case Heirarchy::Root:           return false;
	case Heirarchy::ObjectList:  	return true;
	case Heirarchy::ObjectContents:	return true;
	case Heirarchy::Material:		return GetModel()[(int)property].type != TokType::Texture;
	case Heirarchy::Animation:		return true;
	case Heirarchy::Attachment:		return false;
	case Heirarchy::Textures:		return false;
	default:						break;
	}

	return false;
}

void SpriteModel::activated(const QModelIndex & index )
{
	Entity entity = index.internalPointer();

	if(!index.isValid())
		return;

	if(IsAnimation(entity.heirarchy, entity.property))
		window->ui->viewWidget->set_animation();
	else
		window->ui->viewWidget->need_repaint();
}

bool  SpriteModel::doubleClicked(QModelIndex const& index)
{
typedef fx::gltf::Material::AlphaMode AlphaMode;

	Entity entity = index.internalPointer();
	auto doc = window->document.get();

	if((uint32_t)entity.object >= doc->objects.size())
		return false;

	auto & material = *doc->objects[entity.object]->material;
	auto   detail   = GetModel()[(int)entity.property];

	switch(detail.type)
	{
	case TokType::Boolean:
		doc->addCommand<UpdateMaterialCommand<bool>>(entity.object, detail.offsetof, !Get<bool>(material, detail));
		return true;
	case TokType::AlphaMode:
	{
		AlphaMode mode = Get<AlphaMode>(material, detail);
		mode = (AlphaMode)(((int)mode + 1) % 3);

		doc->addCommand<UpdateMaterialCommand<AlphaMode>>(entity.object, detail.offsetof, mode);
		return true;
	}
	case TokType::Texture:
	{
		std::string path = window->GetImage();

		if(path.empty())
		{
			if(Get<counted_ptr<Image>>(material, detail) == nullptr)
				return false;

			auto r =  QMessageBox::question(window, "Cheetah", "Remove Sprite Sheet?");
			if(r == QMessageBox::No || r== QMessageBox::Cancel)
				return false;
		}

		try
		{
			auto image = Image::Factory(doc->imageManager, path);

			if(image)
			{
				auto error = doc->objects[entity.object]->IsImageCompatible(image, GetTexture(entity.property));

				if(!error.empty())
				{
					window->DisplayError(error);
					return false;
				}
			}

			doc->addCommand<UpdateMaterialCommand<counted_ptr<Image>>>(entity.object, detail.offsetof, image);
			return true;
		}
		catch(std::exception & e)
		{
			window->DisplayError(e.what());
		}
	} return false;
	default:
		break;
	}

	return false;
}


Qt::ItemFlags SpriteModel::flags(QModelIndex const& index) const
{
	Entity entity = index.internalPointer();
	auto doc = window->document.get();

	if(!index.isValid())
		return Qt::NoItemFlags;

	switch(entity.heirarchy)
	{
	case Heirarchy::Root:           return Qt::NoItemFlags;
	case Heirarchy::ObjectList:  	return Qt::ItemIsEnabled;
	case Heirarchy::ObjectContents:	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	case Heirarchy::Material:
	{
		if(index.column() == 0)
		{
			if(GetModel()[(int)entity.property].type == TokType::Texture
			|| GetModel()[(int)entity.property].children == 0)
				return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

			return Qt::ItemIsEnabled;
		}
		else
		{
			if(GetModel()[(int)entity.property].type == TokType::Boolean
			|| GetModel()[(int)entity.property].type == TokType::AlphaMode
			|| GetModel()[(int)entity.property].children != 0)
					return Qt::ItemIsEnabled;

			return Qt::ItemIsEnabled | Qt::ItemIsEditable;
		}
	} break;
	case Heirarchy::Animation:
	{
		if((uint32_t)entity.object >= doc->objects.size())
			return Qt::NoItemFlags;

		if(entity.numeric < 0)
			return Qt::ItemIsEditable | Qt::ItemIsEnabled;
		if(entity.subId < 0)
			return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;

		if(index.column() > 0)
			return Qt::ItemIsEditable | Qt::ItemIsEnabled;
	} break;
	case Heirarchy::Attachment:
	{
		if((uint32_t)entity.object >= doc->objects.size())
			return Qt::NoItemFlags;

		if(entity.numeric < 0)
			return Qt::ItemIsEditable | Qt::ItemIsEnabled;

		return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	} break;
	case Heirarchy::Textures: return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	default:
		break;
	}

	return Qt::NoItemFlags;
}

QString SpriteModel::StringFromVector(shared_array<uint16_t> const& array)
{
	QString r;

	for(uint32_t i = 0; i < array.size(); ++i)
	{
		r += QString::number(array[i]) + " ";
	}

	return r;
}

bool SpriteModel::VectorFromString(shared_array<uint16_t> & dst, QString const& string)
{
	bool okay;

	auto tokens = string.split(' ', Qt::SkipEmptyParts);
	shared_array<uint16_t> eax(tokens.size());

	for(int i = 0; i < tokens.size(); ++i)
	{
		eax[i] = tokens[i].toFloat(&okay);
		if(!okay) return false;
	}

	dst = eax;
	return true;
}

glm::vec2 SpriteModel::GetItemSize(QModelIndex const& index) const
{
	Entity entity = index.internalPointer();

	auto doc = window->document.get();

	if(!index.isValid()
	|| doc == nullptr
	|| (uint32_t)entity.object >= doc->objects.size())
		return {0,0};

	return doc->objects[entity.object]->GetSize();
}

void SpriteModel::Render(GLViewWidget * gl, QModelIndex const& index)
{
	Entity entity = index.internalPointer();
	auto doc = window->document.get();

	if(!index.isValid())
		return;

	switch(entity.heirarchy)
	{
	case Heirarchy::Root:           return;
	case Heirarchy::ObjectList:  	return;
	case Heirarchy::ObjectContents:
		if((uint32_t)entity.object < doc->objects.size())
			doc->objects[entity.object]->RenderObjectSheet(gl);

		return;
	case Heirarchy::Material:
	{
		if((uint32_t)entity.object >= doc->objects.size())
			return;

		auto obj = doc->objects[entity.object].get();
		if(GetModel()[(int)entity.property].type == TokType::Texture)
			obj->RenderSpriteSheet(gl, GetTexture(entity.property), -1);
		else
			obj->RenderObjectSheet(gl, -1);
	} return;
	case Heirarchy::Animation:
	{
		if((uint32_t)entity.object >= doc->objects.size())
			return;

		auto obj = doc->objects[entity.object].get();

		if((uint32_t)entity.numeric >= obj->animations.size())
			obj->RenderObjectSheet(gl);
		else
			doc->RenderAnimation(gl, obj, entity.numeric);

	//	window->ui->viewWidget->need_repaint();
	} return;
	case Heirarchy::Attachment:
	{
		if((uint32_t)entity.object >= doc->objects.size())
			return;

		auto obj = doc->objects[entity.object].get();

//render albedo
		obj->RenderSpriteSheet(gl, GetTexture(MaterialProperty::BaseColorTexture));
		obj->RenderAttachments(gl, entity.numeric);
	}	return;
	case Heirarchy::Textures:
		doc->RenderPackedTextures(gl, entity.object);
		return;
	default:
		break;
	}
}
