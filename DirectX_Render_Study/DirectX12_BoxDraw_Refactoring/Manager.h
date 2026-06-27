#pragma once
#include "Object.h"
class Manager : public CObject
{
public:
    Manager();
    Manager(String _Name);

    virtual ~Manager(){}

    virtual void Update();
    virtual void LateUpdate();
    virtual void Draw();
};

