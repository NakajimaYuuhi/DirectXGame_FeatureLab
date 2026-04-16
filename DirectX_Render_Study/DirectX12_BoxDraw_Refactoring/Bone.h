#pragma once
#include <DirectXMath.h>
#include "StringAlias.h"
#include "ContainerAlias.h"

//ボーンの情報
class CBone
{
public:
    string name;

    int parentIndex; // -1ならルート
    Vector<int> children;

    DirectX::XMMATRIX localBindPose;     // 初期ローカル
    DirectX::XMMATRIX globalBindPose;    // 初期グローバル
    DirectX::XMMATRIX inverseBindPose;   // 逆行列（超重要）

    DirectX::XMMATRIX localPose;   // 現在のローカル（アニメーション）
    DirectX::XMMATRIX globalPose;  // 親子伝播後
};

