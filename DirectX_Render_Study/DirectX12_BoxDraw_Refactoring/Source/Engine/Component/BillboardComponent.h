#pragma once
#include "Component.h"
#include <directxmath.h>

class BillboardComponent : public CComponent
{
public:
	BillboardComponent();
	~BillboardComponent() override = default;

	void LateUpdate(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::PostPhysics; }

	void SetLockYAxis(bool lock) { m_lockYAxis = lock; }
	bool GetLockYAxis() const { return m_lockYAxis; }

private:
	bool m_lockYAxis = false;
};
