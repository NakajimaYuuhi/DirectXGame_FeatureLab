//Object.h
//オブジェクトのクラス
//コンポネントの追加、削除のメソッド有り

//TODO : 複数コンポーネントの扱い
//複数あったとしても安全に扱いたい
//名前で識別できる想定で一旦Nameの変数は作ってある

//TODO : Args,std::forwardの理解が必須

//===== インクルード =====
#pragma once

//基本機能
#include <string>
#include <vector>
#include <memory>



//===== エイリアス宣言 =====

//Uniqueポインタ
template<typename T>
using UniquePtr = std::unique_ptr<T>;

//vector
template<typename T>
using Vector = std::vector<T>;

//string
using String = std::string;



////===== 前方宣言 =====
#include "Component.h"



//===== クラス定義 =====
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

	// Collision callback
	virtual void OnCollision(CObject* _Other);

	// Component lifecycle dispatch helpers (sorted by UpdatePhase)
	void AwakeComponents();
	void StartComponents();
	void UpdateComponents(float deltaTime);
	void LateUpdateComponents(float deltaTime);
	void CollisionComponents(CObject* _Other);

protected:
	//コンポーネント
	Vector<UniquePtr<CComponent>> components;

	//有効、無効フラグ
	bool isValid;
	bool IsDestroyed = false;
	bool m_isVisible = true;
	bool m_hasAwoken = false;
	bool m_hasStarted = false;


	//----- コンポーネントの管理
public:

	// --コンポーネントの取得
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

	// --コンポーネントの追加
	//引数の数、型に制限が無いはず

	//TODO : Args,std::forwardの理解が必須

	template<class T, class... Args>
	T* AddComponent(Args&&... args) 
	{
		auto comp = std::make_unique<T>(std::forward<Args>(args)...);
		comp->SetOwner(this);

		T* raw = comp.get();
		components.push_back(std::move(comp));
		return raw;
	}

public:
	//----- Getter,Setter -----

	// --IsValid
	bool GetIsValid() const			{ return isValid; }
	void SetIsValid(bool _IsValid)	{ isValid = _IsValid; }

	// --IsDestroyed
	bool GetIsDestroyed() const { return IsDestroyed; }

	// --IsVisible
	bool GetIsVisible() const { return m_isVisible; }
	void SetVisible(bool visible) { m_isVisible = visible; }

	// --HasAwoken / HasStarted
	bool GetHasAwoken() const { return m_hasAwoken; }
	void SetHasAwoken(bool awoken) { m_hasAwoken = awoken; }

	bool GetHasStarted() const { return m_hasStarted; }
	void SetHasStarted(bool started) { m_hasStarted = started; }

	const Vector<UniquePtr<CComponent>>& GetComponents() const { return components; }
	void SetIsDestroyed(bool _IsDestroyed) { IsDestroyed = _IsDestroyed; }

	//オブジェクトの名前のセット
	void SetName(String _ObjectName);



};

