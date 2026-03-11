//======================================
//Component.h
//
//======================================
//
//コンポーネントのベースクラス
//識別子(文字列),Tag(必要なら),有効/無効フラグは持つ
//
//======================================

//===== インクルード =====
#pragma once
#include <string>

using String = std::string;

//===== 前方宣言 =====
class CObject;

//===== クラス定義 =====
class CComponent
{
public:

    //コンストラクタ
    CComponent():m_ComponentName(""), m_ComponentIsValid(true),m_Owner(nullptr) {};
    CComponent(String _Name) :m_ComponentName(_Name), m_ComponentIsValid(true), m_Owner(nullptr) {};    //名前を付けれる

    //デストラクタ
    virtual ~CComponent() = default;

    //初期化処理
    virtual void Init(){}

    //----- Getter -----
    String GetName() const { return m_ComponentName; }
    bool GetIsVarid() const { return m_ComponentIsValid; }

    //----- Setter -----
    virtual void SetName(String _Name)      { m_ComponentName = _Name; }
    virtual void SetIsValid(bool _IsValid)  { m_ComponentIsValid = _IsValid; }
    void SetOwner(CObject* _Object) { m_Owner = _Object; }

protected:
    CObject*    m_Owner;
    String      m_ComponentName = "";    // コンポーネントの名前
    bool        m_ComponentIsValid;      //有効/無効フラグ
};

