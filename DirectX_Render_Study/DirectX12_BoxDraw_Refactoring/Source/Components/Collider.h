#pragma once
#include "Component.h"
#include "CollisionLayers.h"
#include <cstdint>

class Collider : public CComponent
{
public:
	typedef enum
	{
		CIRCLE_2D,
		CIRCLE_2D_Trigger,
		CIRCLE_3D,
		CIRCLE_3D_Trigger,
		BOX_2D,
		BOX_2D_Trigger,
		BOX_3D,
		BOX_3D_Trigger,
	}CololiderType;

	Collider() : m_layer(CollisionLayer::Default), m_collisionMask(CollisionLayer::All), m_isTrigger(false), IsTrriger(false) {}
	virtual ~Collider() = default;

	// Type getter
	virtual CololiderType GetColliderType() { return ColliderType; }

	// Trigger property
	bool GetIsTrigger() const { return m_isTrigger; }
	void SetIsTrigger(bool trigger) { m_isTrigger = trigger; IsTrriger = trigger; }

	// Layer & Mask
	uint32_t GetLayer() const { return m_layer; }
	void SetLayer(uint32_t layer) { m_layer = layer; }

	uint32_t GetCollisionMask() const { return m_collisionMask; }
	void SetCollisionMask(uint32_t mask) { m_collisionMask = mask; }

	bool CanCollideWith(uint32_t otherLayer) const { return (m_collisionMask & otherLayer) != 0; }

	// Backward compatibility field
	bool IsTrriger = false;

protected:
	CololiderType ColliderType;
	uint32_t      m_layer = CollisionLayer::Default;
	uint32_t      m_collisionMask = CollisionLayer::All;
	bool          m_isTrigger = false;
};
