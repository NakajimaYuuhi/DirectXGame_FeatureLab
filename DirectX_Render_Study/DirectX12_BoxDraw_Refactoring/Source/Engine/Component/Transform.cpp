#include "Transform.h"
#include <cmath>

using namespace DirectX;

CTransform::CTransform(
    DirectX::XMFLOAT3 _Position,
    DirectX::XMFLOAT3 _Rotation,
    DirectX::XMFLOAT3 _Scale)
    : CComponent("Transform")
    , m_localPosition(_Position)
    , m_localEulerAngles(_Rotation)
    , m_localScale(_Scale)
    , m_isDirty(true)
{
    // オイラー角（ラジアン）から初期クォータニオンを生成
    XMVECTOR q = XMQuaternionRotationRollPitchYaw(m_localEulerAngles.x, m_localEulerAngles.y, m_localEulerAngles.z);
    XMStoreFloat4(&m_localRotation, q);
}

CTransform::~CTransform()
{
    // 既存の親から自身を切り離す
    if (m_parent)
    {
        m_parent->RemoveChild(this);
        m_parent = nullptr;
    }

    // 子ノードの親参照をクリアし、子側もDirtyにする
    for (auto* child : m_children)
    {
        if (child)
        {
            child->m_parent = nullptr;
            child->SetDirty();
        }
    }
    m_children.clear();
}

void CTransform::SetParent(CTransform* newParent, bool keepWorldTransform)
{
    if (m_parent == newParent) return;

    // 循環参照のガード（自分自身、または自分の子孫を親にしようとした場合は無視）
    if (newParent == this || (newParent && newParent->IsChildOf(this)))
    {
        return;
    }

    // ワールド姿勢を維持する場合のローカル座標逆算
    if (keepWorldTransform)
    {
        XMMATRIX currentWorld = GetWorldMatrix();

        if (newParent)
        {
            XMMATRIX parentWorld = newParent->GetWorldMatrix();
            XMVECTOR det;
            XMMATRIX invParent = XMMatrixInverse(&det, parentWorld);
            XMMATRIX newLocal = currentWorld * invParent;

            XMVECTOR scale, rotQuat, trans;
            if (XMMatrixDecompose(&scale, &rotQuat, &trans, newLocal))
            {
                XMStoreFloat3(&m_localScale, scale);
                XMStoreFloat4(&m_localRotation, rotQuat);
                XMStoreFloat3(&m_localPosition, trans);
                UpdateEulerFromQuaternion();
            }
        }
        else
        {
            // 親を解除してルートに戻る場合: 今のワールド姿勢がそのままローカル姿勢になる
            XMVECTOR scale, rotQuat, trans;
            if (XMMatrixDecompose(&scale, &rotQuat, &trans, currentWorld))
            {
                XMStoreFloat3(&m_localScale, scale);
                XMStoreFloat4(&m_localRotation, rotQuat);
                XMStoreFloat3(&m_localPosition, trans);
                UpdateEulerFromQuaternion();
            }
        }
    }

    // 古い親のリストから自身を除外
    if (m_parent)
    {
        m_parent->RemoveChild(this);
    }

    // 新しい親の登録
    m_parent = newParent;
    if (m_parent)
    {
        m_parent->m_children.push_back(this);
    }

    SetDirty();
}

bool CTransform::IsChildOf(const CTransform* potentialParent) const
{
    if (!potentialParent) return false;
    const CTransform* current = m_parent;
    while (current)
    {
        if (current == potentialParent) return true;
        current = current->m_parent;
    }
    return false;
}

void CTransform::RemoveChild(CTransform* child)
{
    auto it = std::remove(m_children.begin(), m_children.end(), child);
    if (it != m_children.end())
    {
        m_children.erase(it, m_children.end());
    }
}

void CTransform::SetLocalPosition(const XMFLOAT3& pos)
{
    m_localPosition = pos;
    SetDirty();
}

void CTransform::SetLocalEulerAngles(const XMFLOAT3& eulerRadians)
{
    m_localEulerAngles = eulerRadians;
    XMVECTOR q = XMQuaternionRotationRollPitchYaw(eulerRadians.x, eulerRadians.y, eulerRadians.z);
    XMStoreFloat4(&m_localRotation, q);
    SetDirty();
}

void CTransform::SetLocalEulerAnglesDegrees(const XMFLOAT3& eulerDegrees)
{
    float pitch = XMConvertToRadians(eulerDegrees.x);
    float yaw   = XMConvertToRadians(eulerDegrees.y);
    float roll  = XMConvertToRadians(eulerDegrees.z);
    SetLocalEulerAngles({ pitch, yaw, roll });
}

DirectX::XMFLOAT3 CTransform::GetLocalEulerAnglesDegrees() const
{
    return {
        XMConvertToDegrees(m_localEulerAngles.x),
        XMConvertToDegrees(m_localEulerAngles.y),
        XMConvertToDegrees(m_localEulerAngles.z)
    };
}

void CTransform::SetLocalRotation(const XMFLOAT4& quat)
{
    m_localRotation = quat;
    UpdateEulerFromQuaternion();
    SetDirty();
}

void CTransform::SetLocalScale(const XMFLOAT3& scale)
{
    m_localScale = scale;
    SetDirty();
}

void CTransform::SetDirty() const
{
    if (m_isDirty) return; // 既にDirtyなら子孫への再帰は不要
    m_isDirty = true;
    for (auto* child : m_children)
    {
        if (child)
        {
            child->SetDirty();
        }
    }
}

void CTransform::UpdateWorldMatrix() const
{
    if (!m_isDirty) return;

    // SRT順: S * R * T
    XMMATRIX S = XMMatrixScaling(m_localScale.x, m_localScale.y, m_localScale.z);
    XMVECTOR rotQuat = XMLoadFloat4(&m_localRotation);
    XMMATRIX R = XMMatrixRotationQuaternion(rotQuat);
    XMMATRIX T = XMMatrixTranslation(m_localPosition.x, m_localPosition.y, m_localPosition.z);

    XMMATRIX localMatrix = S * R * T;

    if (m_parent)
    {
        // 子のローカル変換 * 親のワールド変換
        m_worldMatrix = localMatrix * m_parent->GetWorldMatrix();
    }
    else
    {
        m_worldMatrix = localMatrix;
    }

    m_isDirty = false;
}

XMMATRIX CTransform::GetWorldMatrix() const
{
    if (m_isDirty)
    {
        UpdateWorldMatrix();
    }
    return m_worldMatrix;
}

XMFLOAT3 CTransform::GetWorldPosition() const
{
    XMMATRIX mat = GetWorldMatrix();
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, mat.r[3]);
    return pos;
}

XMFLOAT4 CTransform::GetWorldRotation() const
{
    XMMATRIX mat = GetWorldMatrix();
    XMVECTOR scale, rotQuat, trans;
    if (XMMatrixDecompose(&scale, &rotQuat, &trans, mat))
    {
        XMFLOAT4 q;
        XMStoreFloat4(&q, rotQuat);
        return q;
    }
    return { 0.0f, 0.0f, 0.0f, 1.0f };
}

XMFLOAT3 CTransform::GetWorldScale() const
{
    XMMATRIX mat = GetWorldMatrix();
    XMVECTOR scale, rotQuat, trans;
    if (XMMatrixDecompose(&scale, &rotQuat, &trans, mat))
    {
        XMFLOAT3 s;
        XMStoreFloat3(&s, scale);
        return s;
    }
    return { 1.0f, 1.0f, 1.0f };
}

DirectX::XMFLOAT3 CTransform::GetFront() const
{
    XMMATRIX world = GetWorldMatrix();
    XMVECTOR zAxis = XMVector3Normalize(world.r[2]);
    XMFLOAT3 front;
    XMStoreFloat3(&front, zAxis);
    return front;
}

DirectX::XMFLOAT3 CTransform::GetUp() const
{
    XMMATRIX world = GetWorldMatrix();
    XMVECTOR yAxis = XMVector3Normalize(world.r[1]);
    XMFLOAT3 up;
    XMStoreFloat3(&up, yAxis);
    return up;
}

DirectX::XMFLOAT3 CTransform::GetRight() const
{
    XMMATRIX world = GetWorldMatrix();
    XMVECTOR xAxis = XMVector3Normalize(world.r[0]);
    XMFLOAT3 right;
    XMStoreFloat3(&right, xAxis);
    return right;
}

void CTransform::SetRotationFromUpFront(DirectX::XMFLOAT3 _Up, DirectX::XMFLOAT3 _Front)
{
    // Right の外積計算
    DirectX::XMFLOAT3 right;
    right.x = _Up.y * _Front.z - _Up.z * _Front.y;
    right.y = _Up.z * _Front.x - _Up.x * _Front.z;
    right.z = _Up.x * _Front.y - _Up.y * _Front.x;

    // 正規化
    XMVECTOR r = XMVector3Normalize(XMLoadFloat3(&right));
    XMVECTOR u = XMVector3Normalize(XMLoadFloat3(&_Up));
    XMVECTOR f = XMVector3Normalize(XMLoadFloat3(&_Front));

    XMMATRIX rotMat = XMMatrixIdentity();
    rotMat.r[0] = r;
    rotMat.r[1] = u;
    rotMat.r[2] = f;

    XMVECTOR q = XMQuaternionRotationMatrix(rotMat);
    XMStoreFloat4(&m_localRotation, q);
    UpdateEulerFromQuaternion();
    SetDirty();
}

void CTransform::UpdateEulerFromQuaternion()
{
    XMVECTOR q = XMLoadFloat4(&m_localRotation);
    XMMATRIX rotMat = XMMatrixRotationQuaternion(q);

    // RollPitchYaw に対応するオイラー角の抽出
    float pitch = asinf(-rotMat.r[2].m128_f32[1]);
    float yaw = 0.0f;
    float roll = 0.0f;
    if (cosf(pitch) > 0.0001f)
    {
        yaw  = atan2f(rotMat.r[2].m128_f32[0], rotMat.r[2].m128_f32[2]);
        roll = atan2f(rotMat.r[0].m128_f32[1], rotMat.r[1].m128_f32[1]);
    }
    else
    {
        yaw  = atan2f(-rotMat.r[0].m128_f32[2], rotMat.r[0].m128_f32[0]);
        roll = 0.0f;
    }
    m_localEulerAngles = { pitch, yaw, roll };
}