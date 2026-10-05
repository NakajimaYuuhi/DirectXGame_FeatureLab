#pragma once
#include <DirectXMath.h>
#include <vector>
#include <algorithm>
#include "Component.h"

//===== CTransform クラス定義 =====
class CTransform : public CComponent
{
private:
    // ----- ローカル姿勢（操作対象） -----
    DirectX::XMFLOAT3 m_localPosition    = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_localEulerAngles = { 0.0f, 0.0f, 0.0f }; // ラジアン (Pitch, Yaw, Roll)
    DirectX::XMFLOAT4 m_localRotation    = { 0.0f, 0.0f, 0.0f, 1.0f }; // クォータニオン (x, y, z, w)
    DirectX::XMFLOAT3 m_localScale       = { 1.0f, 1.0f, 1.0f };

    // UV設定（既存機能との互換保持）
    DirectX::XMFLOAT2 m_UVOffset         = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 m_UVScale          = { 1.0f, 1.0f };

    // ----- キャッシュ & 遅延評価 -----
    mutable DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity();
    mutable bool m_isDirty = true;

    // ----- シーングラフ（階層構造） -----
    CTransform* m_parent = nullptr;
    std::vector<CTransform*> m_children;

public:
    // コンストラクタ
    CTransform(
        DirectX::XMFLOAT3 _Position = { 0.0f, 0.0f, 0.0f },
        DirectX::XMFLOAT3 _Rotation = { 0.0f, 0.0f, 0.0f },
        DirectX::XMFLOAT3 _Scale    = { 1.0f, 1.0f, 1.0f }
    );

    // デストラクタ
    virtual ~CTransform();

    // -------------------------------------------------------------
    // 親子関係管理
    // -------------------------------------------------------------
    void SetParent(CTransform* newParent, bool keepWorldTransform = false);
    CTransform* GetParent() const { return m_parent; }
    const std::vector<CTransform*>& GetChildren() const { return m_children; }
    bool IsChildOf(const CTransform* potentialParent) const;

    // -------------------------------------------------------------
    // ローカルプロパティ設定 / 取得（新規API）
    // -------------------------------------------------------------
    void SetLocalPosition(const DirectX::XMFLOAT3& pos);
    void SetLocalEulerAngles(const DirectX::XMFLOAT3& eulerRadians);
    void SetLocalEulerAnglesDegrees(const DirectX::XMFLOAT3& eulerDegrees);
    void SetLocalRotation(const DirectX::XMFLOAT4& quat);
    void SetLocalScale(const DirectX::XMFLOAT3& scale);

    const DirectX::XMFLOAT3& GetLocalPosition() const { return m_localPosition; }
    const DirectX::XMFLOAT3& GetLocalEulerAngles() const { return m_localEulerAngles; }
    DirectX::XMFLOAT3 GetLocalEulerAnglesDegrees() const;
    const DirectX::XMFLOAT4& GetLocalRotation() const { return m_localRotation; }
    const DirectX::XMFLOAT3& GetLocalScale() const { return m_localScale; }

    // -------------------------------------------------------------
    // ワールド行列・座標取得（遅延評価）
    // -------------------------------------------------------------
    DirectX::XMMATRIX GetWorldMatrix() const;
    DirectX::XMFLOAT3 GetWorldPosition() const;
    DirectX::XMFLOAT4 GetWorldRotation() const;
    DirectX::XMFLOAT3 GetWorldScale() const;
    void UpdateWorldMatrix() const;

    // -------------------------------------------------------------
    // 方向ベクトル（親の姿勢も反映したワールド向き）
    // -------------------------------------------------------------
    DirectX::XMFLOAT3 GetFront() const;
    DirectX::XMFLOAT3 GetUp() const;
    DirectX::XMFLOAT3 GetRight() const;

    // 上方向と前方向から回転を設定（ビルボード等で使用）
    void SetRotationFromUpFront(DirectX::XMFLOAT3 _Up, DirectX::XMFLOAT3 _Front);

    // -------------------------------------------------------------
    // 既存コード互換用メソッド（下位互換レイヤー）
    // -------------------------------------------------------------
    DirectX::XMMATRIX GetWorld() { return GetWorldMatrix(); }
    DirectX::XMFLOAT3 GetPos() const { return m_localPosition; }
    void SetPos(const DirectX::XMFLOAT3& _Position) { SetLocalPosition(_Position); }

    DirectX::XMFLOAT3 GetRotation() const { return m_localEulerAngles; }
    void SetRotation(const DirectX::XMFLOAT3& _Rotation) { SetLocalEulerAngles(_Rotation); }

    DirectX::XMFLOAT3 GetScale() const { return m_localScale; }
    void SetScale(const DirectX::XMFLOAT3& _Scale) { SetLocalScale(_Scale); }

    void SetTransform(DirectX::XMFLOAT3 _Position, DirectX::XMFLOAT3 _Scale, DirectX::XMFLOAT3 _Rotation)
    {
        SetLocalPosition(_Position);
        SetLocalScale(_Scale);
        SetLocalEulerAngles(_Rotation);
    }

    DirectX::XMFLOAT2 GetUVOffset() const { return m_UVOffset; }
    void SetUVOffset(DirectX::XMFLOAT2 _UVOffset) { m_UVOffset = _UVOffset; }

    DirectX::XMFLOAT2 GetUVScale() const { return m_UVScale; }
    void SetUVScale(DirectX::XMFLOAT2 _UVScale) { m_UVScale = _UVScale; }

private:
    void SetDirty() const;
    void RemoveChild(CTransform* child);
    void UpdateEulerFromQuaternion();
};

// エイリアス定義
using TransformComponent = CTransform;