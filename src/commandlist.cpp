#include "commandlist.h"

ObjectCommand::ObjectCommand(Document * doc, int obj, counted_string insert) :
	doc(doc),
	new_name(std::move(insert)),
	old_offset(obj),
	new_offset(obj)
{
	if(old_offset < doc->objects.size())
	{
		object   = doc->objects[old_offset];
		old_name = doc->objects[old_offset]->name;
	}

	if(!new_name.empty())
	{
		new_offset = 0;
		for(; new_offset < doc->objects.size(); ++new_offset)
		{
			if(new_name >= doc->objects[new_offset]->name)
				break;
		}

		new_offset -= (!old_name.empty() && new_offset >= old_offset)? 1 : 0;
	}


	if(object == nullptr)
	{
		object = UncountedWrap(new Object(doc->GetViewWidget()));
		object->name = new_name;
	}
}

void ObjectCommand::RollForward()
{
	if(!old_name.empty())
		doc->objects.erase(doc->objects.begin() + old_offset);

	if(!new_name.empty())
		doc->objects.insert(doc->objects.begin() + new_offset, object);
}

void ObjectCommand::RollBack()
{
	if(!new_name.empty())
		doc->objects.erase(doc->objects.begin() + new_offset);

	if(!old_name.empty())
		doc->objects.insert(doc->objects.begin() + old_offset, object);
}

AnimationCommand::AnimationCommand(Document *doc, int obj, int anim, Animation * insert) :
	object(doc->objects[obj])
{
	if((uint32_t)anim < object->animations.size())
		animation = object->animations[anim];

	if(insert == nullptr)
		next = nullptr;
	else
		next_state = *insert;

	if(animation != nullptr)
		prev_state = *animation;
	else if(insert != nullptr)
	{
		animation = UncountedWrap(new Animation(*insert));
		prev = nullptr;
	}
}

void AnimationCommand::RollForward()
{
//insertion command
	if(prev == nullptr)
		InsertAnimation();
	else if(next == nullptr)
		RemoveAnimation();
	else
		UpdateAnimation(next_state);
}

void AnimationCommand::RollBack()
{
//insertion command
	if(prev == nullptr)
		RemoveAnimation();
	else if(next == nullptr)
		InsertAnimation();
	else
		UpdateAnimation(prev_state);
}

void AnimationCommand::InsertAnimation()
{
	for(uint32_t i = 0; i < object->animations.size(); ++i)
	{
		if(object->animations[i]->name >= animation->name)
		{
			object->animations.insert(object->animations.begin()+i, animation);
			return;
		}
	}

	object->animations.push_back(animation);
	return;
}

void AnimationCommand::RemoveAnimation()
{
	for(uint32_t i = 0; i < object->animations.size(); ++i)
	{
		if(object->animations[i]->name == next_state.name)
		{
			object->animations.erase(object->animations.begin()+i);
			return;
		}
	}

	throw std::logic_error("tried to delete animation which does not exist...");
}

void AnimationCommand::UpdateAnimation(Animation const& it)
{
	*animation = it;
	for(uint32_t i = 0; i < object->animations.size(); ++i)
	{
		if(object->animations[i] == animation)
		{
			object->animations.erase(object->animations.begin()+i);
			break;
		}
	}

	for(uint32_t i = 0; i < object->animations.size(); ++i)
	{
		if(object->animations[i] == animation)
		{
			object->animations.insert(object->animations.begin()+i, animation);
			return;
		}
	}

	object->animations.push_back(std::move(animation));
}


AttachmentCommand::AttachmentCommand(Document * doc, int obj, int, std::string insert) :
	object(doc->objects[obj]),
	new_attachment(counted_string::MakeUnique(insert))
{
}

void AttachmentCommand::RollForward()
{

}

void AttachmentCommand::RollBack()
{

}
