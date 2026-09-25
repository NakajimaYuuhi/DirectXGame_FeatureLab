#pragma once
#include <DirectXMath.h>
#include "StringAlias.h"
#include "ContainerAlias.h"

//?{?[??????
class CBone
{
public:
    string name;

    int parentIndex; // -1??à}?[?g
    Vector<int> children;

    DirectX::XMMATRIX localBindPose;     // ???????[?J??
    DirectX::XMMATRIX globalBindPose;    // ?????O???[?o??
    DirectX::XMMATRIX inverseBindPose;   // ?t?s??i???d?v?j

    DirectX::XMMATRIX localPose;   // ???????[?J???i?A?j???[?V?????j
    DirectX::XMMATRIX globalPose;  // ?e?q?`?d??

    // TRS for animation
    DirectX::XMFLOAT3 translation;
    DirectX::XMFLOAT4 rotation; // Quaternion
    DirectX::XMFLOAT3 scale;

    void UpdateLocalPose()
    {
        DirectX::XMVECTOR s = DirectX::XMLoadFloat3(&scale);
        DirectX::XMVECTOR r = DirectX::XMLoadFloat4(&rotation);
        DirectX::XMVECTOR t = DirectX::XMLoadFloat3(&translation);
        localPose = DirectX::XMMatrixAffineTransformation(s, DirectX::XMVectorZero(), r, t);
    }
};
