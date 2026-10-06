#include "ViewportUI.h"
#include "Object.h"
#include "InspectorUI.h"
#include "ContentDrawerUI.h"
#include "D2DTextRenderer.h"
#include <algorithm>
#include <Windows.h>

void CViewportUI::Draw()
{
    if (!m_isVisible) return;

    ImGuiIO& io = ImGui::GetIO();
    float screenW = io.DisplaySize.x;
    float screenH = io.DisplaySize.y;
    float toolbarH = 48.0f;
    float hierarchyW = 320.0f;
    float inspectorW = 460.0f;
    float centerW = screenW - hierarchyW - inspectorW;
    float drawerH = CContentDrawerUI::GetInstance().IsVisible() ? 280.0f : 0.0f;
    float viewportH = screenH - toolbarH - drawerH;

    ImGui::SetNextWindowPos(ImVec2(hierarchyW, toolbarH), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(centerW, viewportH), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    // パディングゼロにして画面端までぴったり合わせる
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("Viewport", &m_isVisible, flags))
    {
        m_isHovered = ImGui::IsWindowHovered();
        m_isFocused = ImGui::IsWindowFocused();
        m_viewportPos = ImGui::GetWindowPos();
        m_viewportSize = ImGui::GetWindowSize();

        ImVec2 contentSize = ImGui::GetContentRegionAvail();

        if (contentSize.x > 0.0f && contentSize.y > 0.0f && m_isTextureAvailable && m_textureSRV.ptr != 0)
        {
            // 16:9 のアスペクト比を維持
            const float targetAspect = 16.0f / 9.0f;
            const float currentAspect = contentSize.x / contentSize.y;

            float renderW = contentSize.x;
            float renderH = contentSize.y;

            if (currentAspect > targetAspect)
            {
                // 横長の場合（ピラーボックス: 左右に余白）
                renderW = contentSize.y * targetAspect;
                renderH = contentSize.y;
            }
            else
            {
                // 縦長の場合（レターボックス: 上下に余白）
                renderW = contentSize.x;
                renderH = contentSize.x / targetAspect;
            }

            // 中央揃えのためのオフセット計算
            float offsetX = (contentSize.x - renderW) * 0.5f;
            float offsetY = (contentSize.y - renderH) * 0.5f;

            ImVec2 cursorPos = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(cursorPos.x + offsetX, cursorPos.y + offsetY));

            m_imagePos = ImGui::GetCursorScreenPos();
            m_imageSize = ImVec2(renderW, renderH);

            // ImTextureID に変換して描画
            ImTextureID texID = static_cast<ImTextureID>(m_textureSRV.ptr);
            ImGui::Image(texID, ImVec2(renderW, renderH));

            // 1. 3Dトランスフォームギズモ描画（Viewportウィンドウ内DrawList）
            CInspectorUI::GetInstance().DrawColliders(ImGui::GetWindowDrawList(), m_imagePos, m_imageSize);
            CInspectorUI::GetInstance().DrawGizmo(ImGui::GetWindowDrawList(), m_imagePos, m_imageSize);

            // 2. 2Dテキストのオーバーレイ描画
            const auto& textQueue = D2DTextRenderer::GetInstance().GetTextQueue();
            if (!textQueue.empty())
            {
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->PushClipRect(m_imagePos, ImVec2(m_imagePos.x + m_imageSize.x, m_imagePos.y + m_imageSize.y), true);

                const float scaleX = renderW / 1920.0f;
                const float scaleY = renderH / 1080.0f;

                for (const auto& info : textQueue)
                {
                    if (info.text.empty()) continue;

                    // UTF-16 -> UTF-8 変換
                    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, info.text.c_str(), (int)info.text.length(), nullptr, 0, nullptr, nullptr);
                    if (sizeNeeded <= 0) continue;

                    std::string utf8Text(sizeNeeded, '\0');
                    WideCharToMultiByte(CP_UTF8, 0, info.text.c_str(), (int)info.text.length(), &utf8Text[0], sizeNeeded, nullptr, nullptr);

                    ImVec2 textPos(m_imagePos.x + info.x * scaleX, m_imagePos.y + info.y * scaleY);
                    ImU32 textColor = ImColor(info.color.r, info.color.g, info.color.b, info.color.a);

                    float scaledFontSize = info.fontSize * scaleY;
                    if (scaledFontSize < 1.0f) scaledFontSize = 1.0f;

                    drawList->AddText(ImGui::GetFont(), scaledFontSize, textPos, textColor, utf8Text.c_str());
                }

                drawList->PopClipRect();
            }
        }
        else
        {
            m_imagePos = m_viewportPos;
            m_imageSize = ImVec2(0.0f, 0.0f);

            // フォールバック表示（テクスチャ未受信時）
            ImVec2 cursorPos = ImGui::GetCursorPos();
            const char* msg = "No Render Texture Available";
            ImVec2 textSize = ImGui::CalcTextSize(msg);
            float offsetX = (contentSize.x - textSize.x) * 0.5f;
            float offsetY = (contentSize.y - textSize.y) * 0.5f;
            if (offsetX < 0.0f) offsetX = 0.0f;
            if (offsetY < 0.0f) offsetY = 0.0f;

            ImGui::SetCursorPos(ImVec2(cursorPos.x + offsetX, cursorPos.y + offsetY));
            ImGui::TextDisabled("%s", msg);
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

bool CViewportUI::GetNormalizedMousePos(float& outX, float& outY) const
{
    if (m_imageSize.x <= 0.0f || m_imageSize.y <= 0.0f)
    {
        return false;
    }

    ImVec2 mousePos = ImGui::GetMousePos();
    float relX = mousePos.x - m_imagePos.x;
    float relY = mousePos.y - m_imagePos.y;

    if (relX >= 0.0f && relX <= m_imageSize.x &&
        relY >= 0.0f && relY <= m_imageSize.y)
    {
        outX = relX / m_imageSize.x;
        outY = relY / m_imageSize.y;
        return true;
    }

    return false;
}
