#pragma once
#include "imgui.h"
#include <cmath>

namespace Theme {

// ─── Color Palette ─────────────────────────────────────────────────────────────
namespace Colors {
    // Backgrounds
    static const ImVec4 BgDeep      = {0.06f, 0.07f, 0.08f, 1.00f};
    static const ImVec4 BgBase      = {0.09f, 0.10f, 0.12f, 1.00f};
    static const ImVec4 BgPanel     = {0.11f, 0.13f, 0.15f, 1.00f};
    static const ImVec4 BgElevated  = {0.14f, 0.16f, 0.19f, 1.00f};
    static const ImVec4 BgHover     = {0.18f, 0.21f, 0.25f, 1.00f};
    static const ImVec4 BgActive    = {0.20f, 0.24f, 0.28f, 1.00f};

    // Borders
    static const ImVec4 Border      = {0.22f, 0.26f, 0.32f, 1.00f};
    static const ImVec4 BorderGlow  = {0.00f, 0.85f, 1.00f, 0.40f};

    // Neon Cyan
    static const ImVec4 Cyan        = {0.00f, 0.85f, 1.00f, 1.00f};
    static const ImVec4 CyanDim     = {0.00f, 0.60f, 0.75f, 1.00f};
    static const ImVec4 CyanGlow    = {0.00f, 0.85f, 1.00f, 0.25f};
    static const ImVec4 CyanDark    = {0.00f, 0.35f, 0.45f, 1.00f};

    // Status
    static const ImVec4 Green       = {0.20f, 0.85f, 0.40f, 1.00f};
    static const ImVec4 GreenDim    = {0.15f, 0.60f, 0.30f, 1.00f};
    static const ImVec4 Orange      = {1.00f, 0.55f, 0.10f, 1.00f};
    static const ImVec4 OrangeDim   = {0.80f, 0.40f, 0.08f, 1.00f};
    static const ImVec4 Red         = {0.95f, 0.25f, 0.25f, 1.00f};
    static const ImVec4 RedDim      = {0.70f, 0.18f, 0.18f, 1.00f};

    // Text
    static const ImVec4 TextPrimary  = {0.92f, 0.94f, 0.96f, 1.00f};
    static const ImVec4 TextSecondary= {0.55f, 0.62f, 0.70f, 1.00f};
    static const ImVec4 TextDisabled = {0.35f, 0.40f, 0.45f, 1.00f};
    static const ImVec4 TextHeading  = {0.95f, 0.97f, 1.00f, 1.00f};
    static const ImVec4 TextCyan     = {0.00f, 0.85f, 1.00f, 1.00f};

    // Transparent
    static const ImVec4 Transparent  = {0.00f, 0.00f, 0.00f, 0.00f};
}

// ─── Apply Full Theme ───────────────────────────────────────────────────────────
inline void Apply() {
    ImGuiStyle& s = ImGui::GetStyle();
    ImVec4* c     = s.Colors;

    // Window
    s.WindowPadding       = {16.0f, 16.0f};
    s.WindowRounding      = 10.0f;
    s.WindowBorderSize    = 1.0f;
    s.WindowMinSize       = {200.0f, 100.0f};
    s.WindowTitleAlign    = {0.5f, 0.5f};

    // Child
    s.ChildRounding       = 8.0f;
    s.ChildBorderSize     = 1.0f;

    // Popup
    s.PopupRounding       = 8.0f;
    s.PopupBorderSize     = 1.0f;

    // Frames
    s.FramePadding        = {10.0f, 6.0f};
    s.FrameRounding       = 6.0f;
    s.FrameBorderSize     = 1.0f;

    // Items
    s.ItemSpacing         = {10.0f, 8.0f};
    s.ItemInnerSpacing    = {8.0f, 6.0f};
    s.IndentSpacing       = 20.0f;
    s.CellPadding         = {8.0f, 5.0f};

    // Scrollbar
    s.ScrollbarRounding   = 6.0f;
    s.ScrollbarSize       = 10.0f;

    // Grab
    s.GrabMinSize         = 8.0f;
    s.GrabRounding        = 4.0f;

    // Tab
    s.TabRounding         = 6.0f;
    s.TabBorderSize       = 1.0f;

    // Separator
    s.SeparatorTextPadding= {14.0f, 3.0f};

    // Alpha
    s.Alpha               = 1.0f;
    s.DisabledAlpha       = 0.45f;

    // Button
    s.ButtonTextAlign     = {0.5f, 0.5f};

    // Colors ──────────────────────────────────────────────────────────────────
    c[ImGuiCol_Text]                  = Colors::TextPrimary;
    c[ImGuiCol_TextDisabled]          = Colors::TextDisabled;
    c[ImGuiCol_WindowBg]              = Colors::BgBase;
    c[ImGuiCol_ChildBg]               = Colors::BgPanel;
    c[ImGuiCol_PopupBg]               = Colors::BgElevated;
    c[ImGuiCol_Border]                = Colors::Border;
    c[ImGuiCol_BorderShadow]          = {0.00f, 0.00f, 0.00f, 0.30f};
    c[ImGuiCol_FrameBg]               = Colors::BgDeep;
    c[ImGuiCol_FrameBgHovered]        = Colors::BgHover;
    c[ImGuiCol_FrameBgActive]         = Colors::BgActive;
    c[ImGuiCol_TitleBg]               = Colors::BgDeep;
    c[ImGuiCol_TitleBgActive]         = {0.07f, 0.09f, 0.12f, 1.00f};
    c[ImGuiCol_TitleBgCollapsed]      = {0.05f, 0.06f, 0.07f, 0.80f};
    c[ImGuiCol_MenuBarBg]             = Colors::BgDeep;
    c[ImGuiCol_ScrollbarBg]           = Colors::BgDeep;
    c[ImGuiCol_ScrollbarGrab]         = Colors::BgHover;
    c[ImGuiCol_ScrollbarGrabHovered]  = Colors::CyanDark;
    c[ImGuiCol_ScrollbarGrabActive]   = Colors::CyanDim;
    c[ImGuiCol_CheckMark]             = Colors::Cyan;
    c[ImGuiCol_SliderGrab]            = Colors::Cyan;
    c[ImGuiCol_SliderGrabActive]      = Colors::CyanDim;
    c[ImGuiCol_Button]                = Colors::BgElevated;
    c[ImGuiCol_ButtonHovered]         = Colors::BgHover;
    c[ImGuiCol_ButtonActive]          = Colors::CyanDark;
    c[ImGuiCol_Header]                = Colors::BgHover;
    c[ImGuiCol_HeaderHovered]         = Colors::CyanDark;
    c[ImGuiCol_HeaderActive]          = Colors::CyanDim;
    c[ImGuiCol_Separator]             = Colors::Border;
    c[ImGuiCol_SeparatorHovered]      = Colors::CyanDim;
    c[ImGuiCol_SeparatorActive]       = Colors::Cyan;
    c[ImGuiCol_ResizeGrip]            = Colors::BgHover;
    c[ImGuiCol_ResizeGripHovered]     = Colors::CyanDim;
    c[ImGuiCol_ResizeGripActive]      = Colors::Cyan;
    c[ImGuiCol_Tab]                   = Colors::BgPanel;
    c[ImGuiCol_TabHovered]            = Colors::CyanDark;
    c[ImGuiCol_TabActive]             = {0.00f, 0.50f, 0.65f, 1.00f};
    c[ImGuiCol_TabUnfocused]          = Colors::BgDeep;
    c[ImGuiCol_TabUnfocusedActive]    = Colors::CyanDark;
    c[ImGuiCol_PlotLines]             = Colors::Cyan;
    c[ImGuiCol_PlotLinesHovered]      = Colors::TextCyan;
    c[ImGuiCol_PlotHistogram]         = Colors::CyanDim;
    c[ImGuiCol_PlotHistogramHovered]  = Colors::Cyan;
    c[ImGuiCol_TableHeaderBg]         = Colors::BgDeep;
    c[ImGuiCol_TableBorderStrong]     = Colors::Border;
    c[ImGuiCol_TableBorderLight]      = {0.15f, 0.18f, 0.22f, 1.00f};
    c[ImGuiCol_TableRowBg]            = {0.00f, 0.00f, 0.00f, 0.00f};
    c[ImGuiCol_TableRowBgAlt]         = {1.00f, 1.00f, 1.00f, 0.03f};
    c[ImGuiCol_TextSelectedBg]        = Colors::CyanGlow;
    c[ImGuiCol_DragDropTarget]        = Colors::Cyan;
    c[ImGuiCol_NavHighlight]          = Colors::Cyan;
    c[ImGuiCol_NavWindowingHighlight] = Colors::Cyan;
    c[ImGuiCol_NavWindowingDimBg]     = {0.00f, 0.00f, 0.00f, 0.50f};
    c[ImGuiCol_ModalWindowDimBg]      = {0.00f, 0.00f, 0.00f, 0.60f};
}

// ─── Animated Pulse ────────────────────────────────────────────────────────────
inline float Pulse(float speed = 2.0f) {
    return (float)(0.5 + 0.5 * std::sin(ImGui::GetTime() * speed));
}

inline ImVec4 LerpColor(const ImVec4& a, const ImVec4& b, float t) {
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    };
}

// ─── Custom Widgets ─────────────────────────────────────────────────────────────
inline void SectionHeader(const char* label, const char* icon = nullptr) {
    float w = ImGui::GetContentRegionAvail().x;
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextCyan);
    if (icon)
        ImGui::Text("%s  %s", icon, label);
    else
        ImGui::Text("%s", label);
    ImGui::PopStyleColor();

    // Draw underline
    ImVec2 p = ImGui::GetCursorScreenPos();
    p.y -= 4.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddLine({p.x, p.y}, {p.x + w, p.y},
                IM_COL32(0, 180, 220, 80), 1.0f);
    ImGui::Dummy({0, 6});
}

inline void GlowSeparator() {
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w  = ImGui::GetContentRegionAvail().x;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilledMultiColor(
        {p.x, p.y}, {p.x + w, p.y + 1.0f},
        IM_COL32(0, 0, 0, 0),
        IM_COL32(0, 180, 220, 120),
        IM_COL32(0, 180, 220, 120),
        IM_COL32(0, 0, 0, 0)
    );
    ImGui::Dummy({0, 4});
}

inline bool CyanButton(const char* label, ImVec2 size = {0, 0}) {
    float p = Pulse(1.5f);
    ImVec4 col = LerpColor(
        {0.00f, 0.45f, 0.60f, 1.0f},
        {0.00f, 0.60f, 0.80f, 1.0f},
        p
    );
    ImGui::PushStyleColor(ImGuiCol_Button,        col);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.00f, 0.75f, 1.00f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0.00f, 0.50f, 0.70f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Text,          {1.0f, 1.0f, 1.0f, 1.0f});
    bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(4);
    return pressed;
}

inline bool PulsingActionButton(const char* label, ImVec2 size = {0, 0}) {
    float p = Pulse(2.5f);
    ImVec4 base = LerpColor(
        {0.00f, 0.55f, 0.20f, 1.0f},
        {0.10f, 0.75f, 0.30f, 1.0f},
        p
    );
    ImGui::PushStyleColor(ImGuiCol_Button,        base);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.20f, 0.90f, 0.40f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0.10f, 0.65f, 0.25f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Text,          {1.0f, 1.0f, 1.0f, 1.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    return pressed;
}

inline void DrawLoadingSpinner(float radius, float thickness, const ImVec4& color) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos.x += radius;
    pos.y += radius;

    float t      = (float)ImGui::GetTime();
    int   numSeg = 24;
    float arcLen = 0.7f; // fraction of circle filled
    float start  = t * 4.0f;

    for (int i = 0; i < numSeg; i++) {
        float a0 = start + (float)i       / numSeg * 6.283f * arcLen;
        float a1 = start + (float)(i + 1) / numSeg * 6.283f * arcLen;
        float fade = (float)i / numSeg;
        ImU32 col = IM_COL32(
            (int)(color.x * 255),
            (int)(color.y * 255),
            (int)(color.z * 255),
            (int)(color.w * 255 * fade)
        );
        dl->PathArcTo(pos, radius, a0, a1, 4);
        dl->PathStroke(col, 0, thickness);
    }
    ImGui::Dummy({radius * 2.0f, radius * 2.0f});
}

inline void AnimatedProgressBar(float fraction, ImVec2 size, const char* overlay = nullptr) {
    float t = Pulse(3.0f);
    ImVec4 col = LerpColor(
        {0.00f, 0.60f, 0.80f, 1.0f},
        {0.00f, 0.85f, 1.00f, 1.0f},
        t
    );
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, col);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::BgDeep);
    ImGui::ProgressBar(fraction, size, overlay);
    ImGui::PopStyleColor(2);
}

inline void StatusLight(bool active, const ImVec4& activeColor, const char* label) {
    ImVec2 p  = ImGui::GetCursorScreenPos();
    float  r  = 5.0f;
    float  t  = Pulse(2.0f);
    ImU32  col = active
        ? IM_COL32(
            (int)((activeColor.x * (0.7f + 0.3f * t)) * 255),
            (int)((activeColor.y * (0.7f + 0.3f * t)) * 255),
            (int)((activeColor.z * (0.7f + 0.3f * t)) * 255),
            255)
        : IM_COL32(60, 65, 75, 255);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddCircleFilled({p.x + r, p.y + r + 2.0f}, r, col);
    if (active) {
        dl->AddCircle({p.x + r, p.y + r + 2.0f}, r + 2.0f,
                      IM_COL32((int)(activeColor.x*255),(int)(activeColor.y*255),(int)(activeColor.z*255), (int)(80*t)), 0, 1.5f);
    }
    ImGui::Dummy({r * 2.0f, r * 2.0f});
    ImGui::SameLine(0, 8.0f);
    ImGui::TextColored(active ? activeColor : Colors::TextDisabled, "%s", label);
}

} // namespace Theme
