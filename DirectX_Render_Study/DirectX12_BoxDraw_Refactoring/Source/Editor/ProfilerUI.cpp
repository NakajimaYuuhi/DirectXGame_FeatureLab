#include "ProfilerUI.h"
#include "Source/Util/Profiler.h"
#include "imgui.h"
#include <vector>
#include <string>
#include <algorithm>

void CProfilerUI::Draw()
{
#ifndef _DEBUG
    return;
#endif

    if (!m_isVisible) return;

    ImGui::SetNextWindowSize(ImVec2(450, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Profiler", &m_isVisible))
    {
        Profiler& profiler = Profiler::GetInstance();
        float fps = profiler.GetCurrentFPS();
        double totalFrameMs = profiler.GetTotalFrameTimeMs();

        // 1. Summary (FPS / Frame Time)
        ImVec4 fpsColor = (fps >= 55.0f) ? ImVec4(0.2f, 0.9f, 0.2f, 1.0f) :
                          (fps >= 30.0f) ? ImVec4(0.9f, 0.8f, 0.2f, 1.0f) :
                                           ImVec4(0.9f, 0.2f, 0.2f, 1.0f);

        ImGui::Text("FPS: ");
        ImGui::SameLine();
        ImGui::TextColored(fpsColor, "%.1f", fps);
        ImGui::SameLine();
        ImGui::Text(" | Frame Time: %.2f ms (%.1f%% of 16.6ms)", totalFrameMs, (totalFrameMs / 16.666) * 100.0);

        // 2. Frame Time History Graph
        const auto& history = profiler.GetFrameTimeHistory();
        if (!history.empty())
        {
            float maxVal = 33.3f;
            for (float val : history)
            {
                if (val > maxVal) maxVal = val;
            }
            char overlayText[64];
            snprintf(overlayText, sizeof(overlayText), "Current: %.2f ms", totalFrameMs);
            ImGui::PlotLines("##FrameHistory", history.data(), static_cast<int>(history.size()),
                             0, overlayText, 0.0f, maxVal, ImVec2(-1, 60.0f));
        }

        ImGui::Separator();

        // 3. Detailed Scope Timings Table
        ImGui::Text("Execution Timings:");

        const auto& samples = profiler.GetSamples();

        if (ImGui::BeginTable("ProfileDetails", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Phase / Scope", ImGuiTableColumnFlags_WidthStretch, 0.40f);
            ImGui::TableSetupColumn("Time (ms)", ImGuiTableColumnFlags_WidthFixed, 65.0f);
            ImGui::TableSetupColumn("Avg (ms)", ImGuiTableColumnFlags_WidthFixed, 65.0f);
            ImGui::TableSetupColumn("Ratio", ImGuiTableColumnFlags_WidthStretch, 0.35f);
            ImGui::TableHeadersRow();

            for (const auto& s : samples)
            {
                ImGui::TableNextRow();

                // Category Coloring
                ImVec4 labelColor = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
                if (s.name.rfind("Update::", 0) == 0)
                {
                    labelColor = ImVec4(0.4f, 0.7f, 1.0f, 1.0f); // Light blue
                }
                else if (s.name.rfind("Draw::", 0) == 0 || s.name.rfind("Render::", 0) == 0)
                {
                    labelColor = ImVec4(1.0f, 0.6f, 0.3f, 1.0f); // Orange
                }
                else if (s.name.rfind("UI::", 0) == 0 || s.name.rfind("ImGui", 0) == 0)
                {
                    labelColor = ImVec4(0.7f, 0.5f, 0.9f, 1.0f); // Purple
                }

                ImGui::TableSetColumnIndex(0);
                ImGui::TextColored(labelColor, "%s", s.name.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.3f", s.durationMs);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.3f", s.smoothedMs);

                ImGui::TableSetColumnIndex(3);
                float ratio = (totalFrameMs > 0.0001) ? static_cast<float>(s.smoothedMs / totalFrameMs) : 0.0f;
                if (ratio > 1.0f) ratio = 1.0f;
                char overlay[32];
                snprintf(overlay, sizeof(overlay), "%.1f%%", ratio * 100.0f);
                ImGui::ProgressBar(ratio, ImVec2(-1, 0), overlay);
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}