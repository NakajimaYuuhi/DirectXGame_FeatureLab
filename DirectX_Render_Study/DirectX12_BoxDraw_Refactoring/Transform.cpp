//インクルード
#include "Transform.h"

//World行列の取得
DirectX::XMMATRIX CTransform::GetWorld() 
{
    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(
        m_Scale.x, m_Scale.y, m_Scale.z);

    DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX(m_Rotation.x);
    DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY(m_Rotation.y);
    DirectX::XMMATRIX rotZ = DirectX::XMMatrixRotationZ(m_Rotation.z);

    DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(
        m_Position.x,
        m_Position.y,
        m_Position.z);

    DirectX::XMMATRIX world =
        scale *
        rotX * rotY * rotZ *
        trans;

    return world;
}
