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

#include "Component.h"
#include "Transform.h"
#include "ObjectInfo.h"
class CRectTransform;

class CObject
{
public:
	CObject();
	CObject(String _Name);
	virtual ~CObject();

	virtual void Init();
	virtual void Awake();
	virtual void Start();
	virtual void Update();
	virtual void LateUpdate();
	virtual void Draw();
	void DrawByLayer(RenderLayer layer);

	virtual void OnCollision(CObject* _Other);

	void AwakeComponents();
	void StartComponents();
	void UpdateComponents(float deltaTime);
	void UpdateComponentsByPhase(UpdatePhase phase, float deltaTime);
	void LateUpdateComponents(float deltaTime);
	void CollisionComponents(CObject* _Other);

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
	CRectTransform* GetRectTransform();

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

		auto comp = std::make_unique<T>(std::forward<Args>(args)...);
		comp->SetOwner(this);

		T* raw = comp.get();
		components.push_back(std::move(comp));
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

	void SetName(String _ObjectName);
	String GetName() const;
};
