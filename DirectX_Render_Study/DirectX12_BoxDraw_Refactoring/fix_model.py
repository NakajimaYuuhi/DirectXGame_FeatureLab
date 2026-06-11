import re

with open('Model.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# I will find the part from m_Bones.clear() until m_Animations = loadedModelData.animations;
old_block = re.search(r'//荳?譌ｦ繝・E繝ｳ繧偵け繝ｪ繧｢.*m_Animations = loadedModelData.animations;', content, re.DOTALL)
if not old_block:
    old_block = re.search(r'//-----.*?// -- 2.ParentData', content, re.DOTALL)

new_block = '''	//----- ボーンデータ作成 -----
	//NodeDataからCBoneのVectorに一括取り込み

	//一旦ボーンをクリア
	m_Bones.clear();

	// -- 1.forループで全データ取り込み
	for (const auto& node : loadedModelData.nodes)
	{
		//領域確保
		auto bone = std::make_shared<CBone>();

		//Name,Childrenを一旦入れる
		bone->name = node.name;
		bone->children = node.children;
		bone->parentIndex = -1; // スキップで埋めるので一旦-1

		// --- ローカル初期行列 (localBindPose) の作成 ---
		
		//行列で持っているかを確認
		bool hasMatrix = false;
		for (int i = 0; i < 16; ++i) { if (node.matrix[i] != 0.0f) { hasMatrix = true; break; } }

		if (hasMatrix)
		{
			bone->localBindPose = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)node.matrix);
		}
		else
		{
			// TRS (初期値) からローカル行列を合成
			DirectX::XMVECTOR s = DirectX::XMVectorSet(node.scale[0], node.scale[1], node.scale[2], 0.0f);
			DirectX::XMVECTOR r = DirectX::XMVectorSet(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]); // クォータニオン
			DirectX::XMVECTOR t = DirectX::XMVectorSet(node.translation[0], node.translation[1], node.translation[2], 0.0f);

			//行列に変換
			bone->localBindPose = DirectX::XMMatrixAffineTransformation(s, DirectX::XMVectorZero(), r, t);
		}

        // Initialize TRS for animation
        DirectX::XMVECTOR s, r, t;
        DirectX::XMMatrixDecompose(&s, &r, &t, bone->localBindPose);
        DirectX::XMStoreFloat3(&bone->scale, s);
        DirectX::XMStoreFloat4(&bone->rotation, r);
        DirectX::XMStoreFloat3(&bone->translation, t);

		// 最初は現在のアニメーション用ローカルポーズ(localPose)を初期姿勢と同じにする
		bone->localPose = bone->localBindPose;

		//プッシュする
		m_Bones.push_back(bone);
	}

	// Copy animations
	m_Animations = loadedModelData.animations;

'''

content = content[:old_block.start()] + new_block + content[old_block.end():]

with open('Model.cpp', 'w', encoding='utf-8') as f:
    f.write(content)
