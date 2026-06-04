#pragma once
#include "3D_Object.h"

#include "StringAlias.h"
class BillBoard : public C3D_Object
{
	public:

	//
	BillBoard(String _Name);
	virtual void Update() override;
	virtual void LateUpdate() override;
	//virtual void Draw() override;

protected:
	String ModelPath;
};

