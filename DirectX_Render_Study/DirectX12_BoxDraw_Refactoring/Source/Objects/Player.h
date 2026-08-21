#pragma once
#include "3D_Object.h"
#include "StringAlias.h"

class Camera;

class Player : public C3D_Object
{
public:
	Player(String _Name);
	~Player() = default;

	void Init() override;
	virtual void Update();

protected:
	String ModelPath;
	float Speed = 0.1f;
	Camera* m_camera = nullptr;
};
