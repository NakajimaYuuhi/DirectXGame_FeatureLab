#pragma once
#include "3D_Object.h"
class Skydome : public C3D_Object
{

public:
	Skydome(String _Name);
	~Skydome() = default;

	//“ü—Í‚ÌXV
	virtual void Update();


protected:
	String ModelPath;
};

