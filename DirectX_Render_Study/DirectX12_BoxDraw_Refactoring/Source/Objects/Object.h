#pragma once
#include <string>
#include <vector>
#include <memory>
#include <type_traits>

template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using Vector = std::vector<T>;

using String = std::string;

#include <DirectXMath.h>
#include "EngineAPI.h"
#include "Component.h"
#include "Transform.h"
#include "ObjectInfo.h"
class CRectTransform;

class CObject
{
public:
	ENGINE_API CObject();
	ENGINE_API CObject(String _Name);
	ENGINE_API virtual ~CObject();

	ENGINE_API virtual void Init();
	ENGINE_API virtual void Awake();
	ENGINE_API virtual void Start();
	ENGINE_API virtual void Update();
	ENGINE_API virtual void LateUpdate();
	ENGINE_API virtual void Draw();
	ENGINE_API void DrawByLayer(RenderLayer layer);
	ENGINE_API void DrawShadow(const DirectX::XMMATRIX& lightViewProj);

	ENGINE_API virtual void OnCollision(CObject* _Other);

	ENGINE_API void AwakeComponents();
	ENGINE_API void StartComponents();
	ENGINE_API void UpdateComponents(float deltaTime);
	ENGINE_API void UpdateComponentsByPhase(UpdatePhase phase, float deltaTime);
	ENGINE_API void LateUpdateComponents(float deltaTime);
	ENGINE_API void CollisionComponents(CObject* _Other);

	// Attach a component instance on the Engine EXE side (safe across DLL boundaries)
	ENGINE_API void AttachComponentRaw(CComponent* comp);

protected:
	Vector<UniquePtr<CComponent>> components;

	bool isValid;
	bool IsDestroyed = false;
	bool m_isVisible = true;
	bool m_hasAwoken = false;
	bool m_hasStarted = false;

public:
	template<class T>
	T* GetComponent() 
	{
		for (auto& c : components) 
		{
			if (auto ptr = dynamic_cast<T*>(c.get())) 
			{
				return ptr;
			}
		}
		return nullptr;
	}

	CTransform* GetTransform() { return GetComponent<CTransform>(); }
	ENGINE_API CRectTransform* GetRectTransform();

	template<class T>
	Vector<T*> GetComponentsOfType()
	{
		Vector<T*> result;
		for (auto& c : components)
		{
			if (auto ptr = dynamic_cast<T*>(c.get()))
			{
				result.push_back(ptr);
			}
		}
		return result;
	}

	template<class T>
	bool RemoveComponent()
	{
		for (auto it = components.begin(); it != components.end(); ++it)
		{
			if (dynamic_cast<T*>(it->get()))
			{
				components.erase(it);
				return true;
			}
		}
		return false;
	}

	bool RemoveComponent(CComponent* targetComp)
	{
		if (!targetComp) return false;
		for (auto it = components.begin(); it != components.end(); ++it)
		{
			if (it->get() == targetComp)
			{
				components.erase(it);
				return true;
			}
		}
		return false;
	}

	template<class T, class... Args>
	T* AddComponent(Args&&... args) 
	{
		if constexpr (std::is_same_v<T, CTransform> || std::is_same_v<T, CObjectInfo>)
		{
			if (T* existing = GetComponent<T>())
			{
				return existing;
			}
		}

		T* raw = new T(std::forward<Args>(args)...);
		AttachComponentRaw(raw);
		return raw;
	}

public:
	bool GetIsValid() const			{ return isValid; }
	void SetIsValid(bool _IsValid)	{ isValid = _IsValid; }

	bool GetIsDestroyed() const { return IsDestroyed; }

	bool GetIsVisible() const { return m_isVisible; }
	void SetVisible(bool visible) { m_isVisible = visible; }

	bool GetHasAwoken() const { return m_hasAwoken; }
	void SetHasAwoken(bool awoken) { m_hasAwoken = awoken; }

	bool GetHasStarted() const { return m_hasStarted; }
	void SetHasStarted(bool started) { m_hasStarted = started; }

	const Vector<UniquePtr<CComponent>>& GetComponents() const { return components; }
	void SetIsDestroyed(bool _IsDestroyed) { IsDestroyed = _IsDestroyed; }

	ENGINE_API void SetName(String _ObjectName);
	ENGINE_API String GetName() const;
};
