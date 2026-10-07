#include "LightComponent.h"
#include "Object.h"
#include "Transform.h"
#include "CameraComponent.h"
#include "DX12Manager.h"
#include "LightManager.h"
#include "imgui.h"
#include <cmath>
#include <algorithm>

namespace
{
	bool ProjectWorldToScreen(
		const DirectX::XMFLOAT3& worldPos,
		const DirectX::XMMATRIX& viewProj,
		float offsetX, float offsetY,
		float screenW, float screenH,
		ImVec2& outScreen)
	{
		DirectX::XMVECTOR v = DirectX::XMVectorSet(worldPos.x, worldPos.y, worldPos.z, 1.0f);
		DirectX::XMVECTOR clip = DirectX::XMVector4Transform(v, viewProj);

		float w = DirectX::XMVectorGetW(clip);
		if (w <= 0.1f) return false;

		float x = DirectX::XMVectorGetX(clip) / w;
		float y = DirectX::XMVectorGetY(clip) / w;

		outScreen.x = offsetX + (x + 1.0f) * 0.5f * screenW;
		outScreen.y = offsetY + (1.0f - y) * 0.5f * screenH;
		return true;
	}
}

LightComponent::LightComponent()
	: CComponent("LightComponent")
{
}

void LightComponent::Init()
{
}

void LightComponent::Start()
{
	SyncToLightManager();
}

void LightComponent::Update(float deltaTime)
{
	if (!IsEnabled()) return;
	SyncToLightManager();
}

std::string LightComponent::GetLightTypeName() const
{
	switch (m_type)
	{
	case LightType::Directional: return "Directional";
	case LightType::Point:       return "Point";
	case LightType::Spot:        return "Spot";
	default:                     return "Unknown";
	}
}

void LightComponent::SyncToLightManager()
{
	if (m_type == LightType::Directional)
	{
		auto& lightMgr = LightManager::GetInstance();

		if (m_Owner)
		{
			CTransform* transform = m_Owner->GetComponent<CTransform>();
			if (transform)
			{
				DirectX::XMFLOAT3 forward = transform->GetFront();
				lightMgr.SetLightDirection(forward);
			}
		}

		lightMgr.SetLightColor(m_color.x, m_color.y, m_color.z, m_intensity);
		lightMgr.SetAmbientColor(m_ambientColor.x, m_ambientColor.y, m_ambientColor.z, m_specularPower);
		lightMgr.SetShadowEnabled(m_castShadow);
		lightMgr.SetShadowBias(m_shadowBias);
		lightMgr.SetShadowDarkness(m_shadowDarkness);
	}
}

void LightComponent::DrawDebug(CameraComponent* camera, ImDrawList* customDrawList, const ImVec2& vpPos, const ImVec2& vpSize)
{
#ifndef _DEBUG
	return;
#endif
	if (!m_Owner) return;

	CTransform* transform = m_Owner->GetComponent<CTransform>();
	if (!transform) return;

	DirectX::XMFLOAT3 pos = transform->GetWorldPosition();
	DirectX::XMMATRIX viewProj = DX12Manager::GetInstance().GetView() * DX12Manager::GetInstance().GetProj();
	ImGuiIO& io = ImGui::GetIO();

	bool useViewport = (vpSize.x > 0.0f && vpSize.y > 0.0f);
	float screenW = useViewport ? vpSize.x : io.DisplaySize.x;
	float screenH = useViewport ? vpSize.y : io.DisplaySize.y;
	float offsetX = useViewport ? vpPos.x : 0.0f;
	float offsetY = useViewport ? vpPos.y : 0.0f;

	ImDrawList* drawList = customDrawList ? customDrawList : ImGui::GetBackgroundDrawList();
	if (!drawList) return;

	if (useViewport)
	{
		drawList->PushClipRect(vpPos, ImVec2(vpPos.x + vpSize.x, vpPos.y + vpSize.y), true);
	}

	// Calculate display color based on light color
	int r = std::clamp(static_cast<int>(m_color.x * 255.0f), 0, 255);
	int g = std::clamp(static_cast<int>(m_color.y * 255.0f), 0, 255);
	int b = std::clamp(static_cast<int>(m_color.z * 255.0f), 0, 255);
	ImU32 lightCol = IM_COL32(r, g, b, 230);
	ImU32 iconBgCol = IM_COL32(20, 20, 20, 180);

	ImVec2 centerScreen;
	if (ProjectWorldToScreen(pos, viewProj, offsetX, offsetY, screenW, screenH, centerScreen))
	{
		// Draw Light Icon / Sun disk
		drawList->AddCircleFilled(centerScreen, 12.0f, iconBgCol);
		drawList->AddCircle(centerScreen, 12.0f, lightCol, 16, 2.0f);

		// Rays around icon
		const int numRays = 8;
		for (int i = 0; i < numRays; ++i)
		{
			float angle = (DirectX::XM_2PI / numRays) * i;
			float cosA = std::cos(angle);
			float sinA = std::sin(angle);
			ImVec2 p1(centerScreen.x + cosA * 15.0f, centerScreen.y + sinA * 15.0f);
			ImVec2 p2(centerScreen.x + cosA * 21.0f, centerScreen.y + sinA * 21.0f);
			drawList->AddLine(p1, p2, lightCol, 1.8f);
		}
	}

	if (m_type == LightType::Directional)
	{
		DirectX::XMFLOAT3 front = transform->GetFront();
		DirectX::XMFLOAT3 right = transform->GetRight();
		DirectX::XMFLOAT3 up    = transform->GetUp();

		float arrowLength = 4.0f;
		DirectX::XMFLOAT3 tipPos = {
			pos.x + front.x * arrowLength,
			pos.y + front.y * arrowLength,
			pos.z + front.z * arrowLength
		};

		ImVec2 pBase, pTip;
		if (ProjectWorldToScreen(pos, viewProj, offsetX, offsetY, screenW, screenH, pBase) &&
			ProjectWorldToScreen(tipPos, viewProj, offsetX, offsetY, screenW, screenH, pTip))
		{
			// Main Direction Arrow
			drawList->AddLine(pBase, pTip, lightCol, 2.8f);

			// Draw 4 fins at arrow tip
			float headSize = 0.8f;
			DirectX::XMFLOAT3 finTargets[4] = {
				{ tipPos.x - front.x * headSize + right.x * headSize * 0.4f,
				  tipPos.y - front.y * headSize + right.y * headSize * 0.4f,
				  tipPos.z - front.z * headSize + right.z * headSize * 0.4f },
				{ tipPos.x - front.x * headSize - right.x * headSize * 0.4f,
				  tipPos.y - front.y * headSize - right.y * headSize * 0.4f,
				  tipPos.z - front.z * headSize - right.z * headSize * 0.4f },
				{ tipPos.x - front.x * headSize + up.x * headSize * 0.4f,
				  tipPos.y - front.y * headSize + up.y * headSize * 0.4f,
				  tipPos.z - front.z * headSize + up.z * headSize * 0.4f },
				{ tipPos.x - front.x * headSize - up.x * headSize * 0.4f,
				  tipPos.y - front.y * headSize - up.y * headSize * 0.4f,
				  tipPos.z - front.z * headSize - up.z * headSize * 0.4f }
			};

			for (int i = 0; i < 4; ++i)
			{
				ImVec2 pFin;
				if (ProjectWorldToScreen(finTargets[i], viewProj, offsetX, offsetY, screenW, screenH, pFin))
				{
					drawList->AddLine(pTip, pFin, lightCol, 2.0f);
				}
			}
		}

		// Multiple parallel rays to visualize directional sunlight coverage
		const float offsetDist = 1.5f;
		DirectX::XMFLOAT3 rayOffsets[4] = {
			{  right.x * offsetDist,  right.y * offsetDist,  right.z * offsetDist },
			{ -right.x * offsetDist, -right.y * offsetDist, -right.z * offsetDist },
			{  up.x * offsetDist,     up.y * offsetDist,     up.z * offsetDist },
			{ -up.x * offsetDist,    -up.y * offsetDist,    -up.z * offsetDist }
		};

		ImU32 secondaryCol = IM_COL32(r, g, b, 120);
		for (int i = 0; i < 4; ++i)
		{
			DirectX::XMFLOAT3 startP = { pos.x + rayOffsets[i].x, pos.y + rayOffsets[i].y, pos.z + rayOffsets[i].z };
			DirectX::XMFLOAT3 endP   = { startP.x + front.x * 2.5f, startP.y + front.y * 2.5f, startP.z + front.z * 2.5f };

			ImVec2 sScr, eScr;
			if (ProjectWorldToScreen(startP, viewProj, offsetX, offsetY, screenW, screenH, sScr) &&
				ProjectWorldToScreen(endP, viewProj, offsetX, offsetY, screenW, screenH, eScr))
			{
				drawList->AddLine(sScr, eScr, secondaryCol, 1.2f);
			}
		}
	}
	else if (m_type == LightType::Point)
	{
		// Draw 3 orthogonal circles representing the point light range sphere
		const int segments = 24;
		ImU32 sphereCol = IM_COL32(r, g, b, 140);

		auto drawCircleAxis = [&](int axis) {
			ImVec2 prevScreen;
			bool prevValid = false;
			ImVec2 firstScreen;
			bool firstValid = false;

			for (int i = 0; i <= segments; ++i)
			{
				float theta = (DirectX::XM_2PI / segments) * i;
				float c = std::cos(theta) * m_range;
				float s = std::sin(theta) * m_range;

				DirectX::XMFLOAT3 point = pos;
				if (axis == 0)      { point.x += c; point.z += s; } // XZ plane
				else if (axis == 1) { point.x += c; point.y += s; } // XY plane
				else                { point.y += c; point.z += s; } // YZ plane

				ImVec2 currScreen;
				bool currValid = ProjectWorldToScreen(point, viewProj, offsetX, offsetY, screenW, screenH, currScreen);

				if (i == 0)
				{
					firstScreen = currScreen;
					firstValid = currValid;
				}

				if (prevValid && currValid)
				{
					drawList->AddLine(prevScreen, currScreen, sphereCol, 1.2f);
				}

				prevScreen = currScreen;
				prevValid = currValid;
			}
		};

		drawCircleAxis(0); // XZ
		drawCircleAxis(1); // XY
		drawCircleAxis(2); // YZ
	}

	if (useViewport)
	{
		drawList->PopClipRect();
	}
}
