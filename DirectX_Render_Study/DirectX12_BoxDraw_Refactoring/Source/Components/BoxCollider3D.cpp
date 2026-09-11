#include "BoxCollider3D.h"
#include "Camera.h"
#include "imgui.h"

void BoxCollider3D::DrawDebug(Camera* camera)
{
#ifndef _DEBUG
	return;
#endif // !_DEBUG

	if (!camera) return;

	DirectX::XMFLOAT3 center = GetWorldPos();
	DirectX::XMFLOAT3 size = GetSize();

	float hx = size.x * 0.5f;
	float hy = size.y * 0.5f;
	float hz = size.z * 0.5f;

	DirectX::XMFLOAT3 corners[8] = {
		{ center.x - hx, center.y - hy, center.z - hz },
		{ center.x + hx, center.y - hy, center.z - hz },
		{ center.x + hx, center.y + hy, center.z - hz },
		{ center.x - hx, center.y + hy, center.z - hz },
		{ center.x - hx, center.y - hy, center.z + hz },
		{ center.x + hx, center.y - hy, center.z + hz },
		{ center.x + hx, center.y + hy, center.z + hz },
		{ center.x - hx, center.y + hy, center.z + hz }
	};

	DirectX::XMMATRIX viewProj = camera->GetView() * camera->GetProj();
	ImGuiIO& io = ImGui::GetIO();
	float screenW = io.DisplaySize.x;
	float screenH = io.DisplaySize.y;

	ImVec2 screenCorners[8];
	bool valid[8];

	for (int i = 0; i < 8; ++i)
	{
		DirectX::XMVECTOR v = DirectX::XMVectorSet(corners[i].x, corners[i].y, corners[i].z, 1.0f);
		DirectX::XMVECTOR clip = DirectX::XMVector4Transform(v, viewProj);

		float w = DirectX::XMVectorGetW(clip);
		if (w > 0.1f)
		{
			float x = DirectX::XMVectorGetX(clip) / w;
			float y = DirectX::XMVectorGetY(clip) / w;

			screenCorners[i].x = (x + 1.0f) * 0.5f * screenW;
			screenCorners[i].y = (1.0f - y) * 0.5f * screenH;
			valid[i] = true;
		}
		else
		{
			valid[i] = false;
		}
	}

	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	if (!drawList) return;

	static const int edges[12][2] = {
		{0,1}, {1,2}, {2,3}, {3,0},
		{4,5}, {5,6}, {6,7}, {7,4},
		{0,4}, {1,5}, {2,6}, {3,7}
	};

	ImU32 debugColor = IM_COL32(0, 255, 0, 255);

	for (int i = 0; i < 12; ++i)
	{
		int idxA = edges[i][0];
		int idxB = edges[i][1];
		if (valid[idxA] && valid[idxB])
		{
			drawList->AddLine(screenCorners[idxA], screenCorners[idxB], debugColor, 2.0f);
		}
	}
}