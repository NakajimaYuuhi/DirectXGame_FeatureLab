#pragma once
#include "ObjectTag.h"
#include "Component.h"
#include <cassert>
#include "StringAlias.h"

class CObjectInfo : public CComponent
{
public:
	CObjectInfo() 
		: objectName("")
		, CComponent("ObjectInfo")
	{}

	CObjectInfo(String _ObjectName)
		: objectName(_ObjectName)
		, CComponent("ObjectInfo")
	{}

private:
	String objectName;
	ObjectTag objectTag = ObjectTag::NONE;
	String prefabName = "";

public:
	void SetIsValid(bool _ComponentIsValid) 
	{ 
		if (!_ComponentIsValid)
		{
			assert(false && "Attempted to disable a locked component / ObjectInfo");
			return; 
		}
		m_ComponentIsValid = _ComponentIsValid;
	}

	void SetObjectName(String _ObjectName) { objectName = _ObjectName; }
	String GetObjectName() const { return objectName; }

	void SetObjectTag(ObjectTag _ObjectTag) { objectTag = _ObjectTag; }
	ObjectTag GetObjectTag() const { return objectTag; }

	void SetPrefabName(const String& _PrefabName) { prefabName = _PrefabName; }
	const String& GetPrefabName() const { return prefabName; }
	bool IsPrefab() const { return !prefabName.empty(); }
};
