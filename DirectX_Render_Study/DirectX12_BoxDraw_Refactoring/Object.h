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
class CComponent;



//===== クラス定義 =====
class CObject
{
public:
	CObject();
	virtual ~CObject();


	virtual void Init();
	virtual void Update() = 0;
	virtual void Draw() = 0;

protected:
	//コンポーネント
	Vector<UniquePtr<CComponent>> m_Components;

	//有効、無効フラグ
	bool m_IsValid;


	//----- コンポーネントの管理
public:

	// --コンポーネントの取得
	template<class T>
	T* GetComponent() 
	{
		//コンテナ内を探索
		for (auto& c : m_Components) 
		{
			//生ポインタ取得
			//キャストができれば、それを返す
			if (auto ptr = dynamic_cast<T*>(c.get())) 
			{
				return ptr;
			}
		}
		return nullptr;
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
		m_Components.push_back(std::move(comp));
		return raw;
	}

public:
	//----- Getter,Setter -----

	// --IsValid
	bool GetIsVarid() const			{ return m_IsValid; }
	void SetIsValid(bool _IsValid)	{ m_IsValid = _IsValid; }

	//オブジェクトの名前のセット
	void SetName(String _ObjectName);

};

