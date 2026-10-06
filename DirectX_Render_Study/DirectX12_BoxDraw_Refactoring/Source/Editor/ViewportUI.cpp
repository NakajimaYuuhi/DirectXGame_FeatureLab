#include "ViewportUI.h"
#include <algorithm>

void CViewportUI::Draw()
{
    if (!m_isVisible) return;

    // パディングをゼロにして画面端までぴったり合わせる
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("Viewport", &m_isVisible))
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
                // 横長すぎる場合（ピラーボックス: 左右に余白）
                renderW = contentSize.y * targetAspect;
                renderH = contentSize.y;
            }
            else
            {
                // 縦長すぎる場合（レターボックス: 上下に余白）
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
