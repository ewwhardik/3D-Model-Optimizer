#include "app.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <chrono>

#ifdef HAVE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

// ─── Helpers ───────────────────────────────────────────────────────────────────
static std::string fmtBytes(size_t b) {
    char buf[64];
    if (b < 1024) snprintf(buf, 64, "%zu B", b);
    else if (b < 1024*1024) snprintf(buf, 64, "%.1f KB", b/1024.0);
    else snprintf(buf, 64, "%.2f MB", b/1024.0/1024.0);
    return buf;
}

static std::string fmtNum(size_t n) {
    // Insert commas
    std::string s = std::to_string(n);
    int ins = (int)s.size() - 3;
    while (ins > 0) { s.insert(ins, ","); ins -= 3; }
    return s;
}

// ─── Constructor / Destructor ──────────────────────────────────────────────────
App::App() {
    m_statusColor = Theme::Colors::TextSecondary;
    m_camera.distance = 4.0f;
}

App::~App() {
    shutdown();
}

bool App::init(GLFWwindow* window) {
    m_window = window;
    glfwGetWindowSize(window, &m_winW, &m_winH);

    Theme::Apply();

    m_renderer.init(800, 600);

    m_panelFade  = 1.0f;
    m_fadeInTimer = 0.0f;

    addLog(LogLevel::Info, "poop3D Compiler initialized");
    addLog(LogLevel::Info, "Ready to import or generate 3D models");
    return true;
}

void App::shutdown() {
    m_cancelOptimize = true;
    if (m_workerThread && m_workerThread->joinable())
        m_workerThread->join();
    m_renderer.shutdown();
}

// ─── Update ────────────────────────────────────────────────────────────────────
void App::update(float dt) {
    m_appTime += dt;
    if (m_fadeInTimer < 1.0f) m_fadeInTimer = std::min(1.0f, m_fadeInTimer + dt * 2.0f);
    if (m_successTimer > 0.0f) m_successTimer -= dt;

    // Banner fade
    if (m_state == AppState::Done && m_successTimer > 0.0f)
        m_bannerAlpha = std::min(1.0f, m_bannerAlpha + dt * 4.0f);
    else
        m_bannerAlpha = std::max(0.0f, m_bannerAlpha - dt * 2.0f);

    // Auto-rotate
    if (m_view.rotating)
        m_camera.yaw += m_view.rotateSpeed * dt * 20.0f;

    // Poll worker thread progress
    if (m_state == AppState::Optimizing) {
        {
            std::lock_guard<std::mutex> lk(m_progressMutex);
            m_progress      = m_threadProgress;
            m_progressStage = m_threadStage;
        }

        if (m_optimizeDone.load()) {
            m_workerThread->join();
            m_workerThread.reset();
            m_optimizeDone = false;

            if (m_optimizeSuccess) {
                m_state        = AppState::Done;
                m_successTimer = 8.0f;
                m_bannerAlpha  = 0.0f;
                m_statusText   = "✔ Optimization complete!";
                m_statusColor  = Theme::Colors::Green;
                m_hasOptimized = true;
                m_showOptimized = true;
                m_renderer.uploadMesh(m_optimizedMesh);
                addLog(LogLevel::Success, "Optimization complete! " +
                    std::to_string((int)m_result.percentageSaved) + "% storage saved");
            } else {
                m_state = AppState::Error;
                m_statusText  = "✘ Optimization failed: " + m_result.errorMsg;
                m_statusColor = Theme::Colors::Red;
                addLog(LogLevel::Error, "Optimization failed: " + m_result.errorMsg);
            }
        }
    }
}

// ─── Main Render ───────────────────────────────────────────────────────────────
void App::render() {
    glfwGetWindowSize(m_window, &m_winW, &m_winH);

    ImGuiIO& io = ImGui::GetIO();

    // Full-screen dockspace background
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({(float)m_winW, (float)m_winH});
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##Root", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDecoration);
    ImGui::End();

    float toolbarH  = 52.0f;
    float statusH   = 28.0f;
    float workspace = (float)m_winH - toolbarH - statusH;

    renderTopToolbar();
    renderStatusBar(statusH);

    // Workspace
    float leftW  = std::max(280.0f, (float)m_winW * 0.27f);
    float rightW = (float)m_winW - leftW;

    // Left: Operations Panel
    ImGui::SetNextWindowPos({0, toolbarH});
    ImGui::SetNextWindowSize({leftW, workspace});
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::Colors::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_Border,   Theme::Colors::Border);
    ImGui::Begin("##OpsPanel", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    renderOperationsPanel(leftW, workspace);
    ImGui::End();
    ImGui::PopStyleColor(2);

    // Right: Analytics + Viewport split
    float analyticsH = workspace * 0.42f;
    float viewportH  = workspace - analyticsH;

    // Viewport (top-right)
    m_viewX = leftW; m_viewY = toolbarH;
    m_viewW = (int)rightW; m_viewH = (int)viewportH;

    ImGui::SetNextWindowPos({leftW, toolbarH});
    ImGui::SetNextWindowSize({rightW, viewportH});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.05f, 0.06f, 0.08f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Border,   Theme::Colors::Border);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::Begin("##Viewport", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    renderViewport(leftW, toolbarH, rightW, viewportH);
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    // Analytics (bottom-right)
    ImGui::SetNextWindowPos({leftW, toolbarH + viewportH});
    ImGui::SetNextWindowSize({rightW, analyticsH});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::Colors::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_Border,   Theme::Colors::Border);
    ImGui::Begin("##Analytics", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar);
    renderAnalyticsPanel(rightW, analyticsH);
    ImGui::End();
    ImGui::PopStyleColor(2);

    // Modals
    if (m_showAbout)    renderAboutModal();
    if (m_showSettings) renderSettingsModal();

    // Success banner overlay
    if (m_bannerAlpha > 0.01f && m_state == AppState::Done)
        renderSuccessBanner();
}

// ─── Top Toolbar ───────────────────────────────────────────────────────────────
void App::renderTopToolbar() {
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({(float)m_winW, 52.0f});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.06f, 0.07f, 0.09f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Border,   Theme::Colors::BorderGlow);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {12.0f, 8.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   {10.0f, 0.0f});

    ImGui::Begin("##Toolbar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Logo
    float pulse = Theme::Pulse(1.5f);
    ImVec4 logoColor = Theme::LerpColor(
        {0.0f, 0.7f, 0.9f, 1.0f},
        {0.2f, 0.95f, 1.0f, 1.0f}, pulse);
    ImGui::PushStyleColor(ImGuiCol_Text, logoColor);
    ImGui::SetWindowFontScale(1.2f);
    ImGui::Text("  \xF0\x9F\x92\xA9  poop3D Compiler");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImGui::SameLine(0, 40.0f);

    // Toolbar buttons
    auto tbBtn = [&](const char* label) -> bool {
        ImGui::PushStyleColor(ImGuiCol_Button,        Theme::Colors::BgElevated);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::Colors::BgHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Theme::Colors::CyanDark);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        bool r = ImGui::Button(label, {0, 32});
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        return r;
    };

    if (tbBtn("  \xF0\x9F\x93\x82  Import")) {
        addLog(LogLevel::Info, "Use drag & drop or Generate Mock Mesh to load a model");
        m_statusText  = "Drag & drop a .OBJ/.FBX/.GLTF/.PLY/.STL file onto the window";
        m_statusColor = Theme::Colors::TextCyan;
    }
    ImGui::SameLine();
    if (tbBtn("  \xF0\x9F\x92\xBE  Export")) {
        if (m_hasOptimized || m_hasOriginal) {
            exportMesh(m_exportPath);
        } else {
            addLog(LogLevel::Warning, "Nothing to export yet — optimize a mesh first");
        }
    }
    ImGui::SameLine();
    if (tbBtn("  \xE2\x9A\x99  Settings")) m_showSettings = true;
    ImGui::SameLine();
    if (tbBtn("  \xE2\x84\xB9  About"))    m_showAbout    = true;

    // Status lights (right-aligned)
    ImGui::SameLine(ImGui::GetWindowWidth() - 340.0f);
    Theme::StatusLight(m_state == AppState::Optimizing, Theme::Colors::Orange, "Processing");
    ImGui::SameLine(0, 20.0f);
    Theme::StatusLight(m_hasOriginal,  Theme::Colors::Cyan,  "Mesh Loaded");
    ImGui::SameLine(0, 20.0f);
    Theme::StatusLight(m_hasOptimized, Theme::Colors::Green, "Optimized");
    ImGui::SameLine(0, 20.0f);
    // FPS counter
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextSecondary);
    ImGui::Text("%.0f FPS", ImGui::GetIO().Framerate);
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

// ─── Status Bar ────────────────────────────────────────────────────────────────
void App::renderStatusBar(float h) {
    ImGui::SetNextWindowPos({0, (float)m_winH - h});
    ImGui::SetNextWindowSize({(float)m_winW, h});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.05f, 0.06f, 0.07f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Border,   Theme::Colors::Border);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {14.0f, 5.0f});

    ImGui::Begin("##StatusBar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::PushStyleColor(ImGuiCol_Text, m_statusColor);
    ImGui::Text("%s", m_statusText.c_str());
    ImGui::PopStyleColor();

    ImGui::SameLine(ImGui::GetWindowWidth() - 280.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextDisabled);
    if (m_hasOriginal)
        ImGui::Text("Vertices: %s  |  Triangles: %s",
            fmtNum(m_originalMesh.vertices.size()).c_str(),
            fmtNum(m_originalMesh.indices.size()/3).c_str());
    else
        ImGui::Text("No mesh loaded");
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
}

// ─── Operations Panel ──────────────────────────────────────────────────────────
void App::renderOperationsPanel(float panelW, float /*panelH*/) {
    // Fade in
    float alpha = std::min(1.0f, m_fadeInTimer * 2.0f);
    ImGui::SetNextWindowBgAlpha(alpha);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8, 10});
    Theme::SectionHeader("OPERATIONS", "\xF0\x9F\x9B\xA0");
    ImGui::Spacing();

    // ── Import ────────────────────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::Colors::BgElevated);
    ImGui::BeginChild("##ImportCard", {panelW - 32, 88}, true);
    Theme::SectionHeader("Import Model", "\xF0\x9F\x93\x82");
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextSecondary);
    ImGui::TextWrapped("Drag & drop OBJ, FBX, GLTF, GLB, PLY, STL onto window");
    ImGui::PopStyleColor();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // ── Generate Mock ─────────────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::Colors::BgElevated);
    ImGui::BeginChild("##MockCard", {panelW - 32, 110}, true);
    Theme::SectionHeader("Generate Test Mesh", "\xE2\x9A\xA1");

    const char* complexItems[] = { "~50K vertices (Fast)", "~250K vertices (Medium)", "~1M vertices (Heavy)" };
    ImGui::SetNextItemWidth(panelW - 60);
    ImGui::Combo("##Complexity", &m_settings.mockComplexity, complexItems, 3);
    ImGui::Spacing();

    bool busy = (m_state == AppState::Optimizing || m_state == AppState::Importing);
    if (busy) ImGui::BeginDisabled();
    if (Theme::CyanButton("  \xE2\x9A\xA1  Generate Mock Mesh  ", {panelW - 48, 32})) {
        generateMockMesh(m_settings.mockComplexity);
    }
    if (busy) ImGui::EndDisabled();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // ── Optimization Settings ─────────────────────────────────────────────────
    renderOptimizationSettings();
    ImGui::Spacing();

    // ── Compression Preset ────────────────────────────────────────────────────
    renderCompressionPresets();
    ImGui::Spacing();

    // ── BIG ACTION BUTTON ─────────────────────────────────────────────────────
    if (busy) ImGui::BeginDisabled();
    if (!m_hasOriginal) ImGui::BeginDisabled();

    ImGui::Spacing();
    if (Theme::PulsingActionButton(
        "  \xE2\x96\xB6  Optimize & Compress  ",
        {panelW - 32, 44}))
    {
        runOptimize();
    }

    if (!m_hasOriginal) ImGui::EndDisabled();
    if (busy) ImGui::EndDisabled();

    if (!m_hasOriginal) {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextDisabled);
        ImGui::TextWrapped("Import or generate a mesh first");
        ImGui::PopStyleColor();
    }

    // ── Progress ──────────────────────────────────────────────────────────────
    if (m_state == AppState::Optimizing) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::Colors::BgElevated);
        ImGui::BeginChild("##ProgressCard", {panelW - 32, 90}, true);

        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextCyan);
        ImGui::Text("\xE2\x9A\x99  Processing...");
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextSecondary);
        ImGui::TextWrapped("%s", m_progressStage.c_str());
        ImGui::PopStyleColor();

        Theme::AnimatedProgressBar(m_progress, {panelW - 64, 18});

        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextSecondary);
        ImGui::Text("%.1f%%", m_progress * 100.0f);
        ImGui::PopStyleColor();

        ImGui::SameLine(panelW - 100);
        ImGui::PushStyleColor(ImGuiCol_Button,        Theme::Colors::RedDim);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::Colors::Red);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Theme::Colors::Red);
        if (ImGui::Button("Cancel", {70, 24})) {
            m_cancelOptimize = true;
            addLog(LogLevel::Warning, "Optimization cancelled by user");
            m_state = AppState::Idle;
        }
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        Theme::DrawLoadingSpinner(10.0f, 2.5f, Theme::Colors::Cyan);

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    // ── View Toggle ───────────────────────────────────────────────────────────
    if (m_hasOptimized) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::Colors::BgElevated);
        ImGui::BeginChild("##ViewToggle", {panelW - 32, 50}, true);
        ImGui::Text("Viewing:");
        ImGui::SameLine();
        if (!m_showOptimized) {
            if (Theme::CyanButton("Original")) m_showOptimized = true;
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextDisabled);
            ImGui::Text("Optimized");
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextDisabled);
            ImGui::Text("Original");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (Theme::CyanButton("Optimized")) {
                // switch
            }
        }
        ImGui::SameLine(panelW - 110);
        if (ImGui::Button("Swap")) {
            m_showOptimized = !m_showOptimized;
            const MeshData& mesh = m_showOptimized ? m_optimizedMesh : m_originalMesh;
            m_renderer.uploadMesh(mesh);
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleVar();
}

// ─── Optimization Settings ─────────────────────────────────────────────────────
void App::renderOptimizationSettings() {
    float pw = ImGui::GetContentRegionAvail().x;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::Colors::BgElevated);
    ImGui::BeginChild("##OptSettings", {pw, 250}, true);

    Theme::SectionHeader("Optimization Settings", "\xF0\x9F\x94\xA7");

    auto chk = [&](const char* label, bool& val, const char* tip = nullptr) {
        ImGui::PushStyleColor(ImGuiCol_CheckMark,     Theme::Colors::Cyan);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,       Theme::Colors::BgDeep);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,Theme::Colors::BgHover);
        bool r = ImGui::Checkbox(label, &val);
        ImGui::PopStyleColor(3);
        if (tip && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", tip);
        return r;
    };

    chk("Remove duplicate vertices",   m_settings.removeDuplicateVertices,
        "Finds and merges exact duplicate vertices");
    chk("Generate index buffer",        m_settings.generateIndexBuffer,
        "Creates indexed geometry for GPU efficiency");
    chk("Weld identical vertices",      m_settings.weldIdenticalVertices,
        "Merges vertices within a small distance threshold");
    chk("Remove unused vertices",       m_settings.removeUnusedVertices,
        "Purges vertices not referenced by any triangle");
    chk("Remove degenerate triangles",  m_settings.removeDegenerateTriangles,
        "Removes zero-area triangles");
    chk("Position quantization",        m_settings.positionQuantization,
        "Reduces position precision to save memory");
    chk("Normal quantization",          m_settings.normalQuantization,
        "Reduces normal precision to 8-bit");
    chk("Optimize vertex cache",        m_settings.optimizeVertexCache,
        "Reorders triangles for GPU cache efficiency");
    chk("Optimize vertex fetch",        m_settings.optimizeVertexFetch,
        "Reorders vertices for sequential fetch");

    if (m_settings.positionQuantization) {
        ImGui::SetNextItemWidth(120);
        ImGui::SliderInt("Quantize bits", &m_settings.quantizationBits, 6, 16);
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ─── Compression Presets ───────────────────────────────────────────────────────
void App::renderCompressionPresets() {
    float pw = ImGui::GetContentRegionAvail().x;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::Colors::BgElevated);
    ImGui::BeginChild("##Presets", {pw, 78}, true);
    Theme::SectionHeader("Compression Preset", "\xF0\x9F\x93\xA6");

    auto preset = [&](const char* name, int idx, const char* desc) {
        bool active = m_settings.preset == idx;
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button,        {0.00f, 0.45f, 0.60f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.00f, 0.60f, 0.80f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0.00f, 0.35f, 0.50f, 1.0f});
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button,        Theme::Colors::BgHover);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::Colors::BgActive);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Theme::Colors::CyanDark);
        }
        if (ImGui::Button(name, {72, 28})) {
            m_settings.preset = idx;
            switch(idx) {
                case 0: // Fast
                    m_settings.removeDuplicateVertices  = true;
                    m_settings.generateIndexBuffer      = true;
                    m_settings.weldIdenticalVertices    = false;
                    m_settings.removeUnusedVertices     = true;
                    m_settings.removeDegenerateTriangles= true;
                    m_settings.positionQuantization     = false;
                    m_settings.normalQuantization       = false;
                    m_settings.optimizeVertexCache      = false;
                    m_settings.optimizeVertexFetch      = false;
                    break;
                case 1: // Balanced
                    m_settings.removeDuplicateVertices  = true;
                    m_settings.generateIndexBuffer      = true;
                    m_settings.weldIdenticalVertices    = true;
                    m_settings.removeUnusedVertices     = true;
                    m_settings.removeDegenerateTriangles= true;
                    m_settings.positionQuantization     = false;
                    m_settings.normalQuantization       = false;
                    m_settings.optimizeVertexCache      = true;
                    m_settings.optimizeVertexFetch      = true;
                    break;
                case 2: // Maximum
                    m_settings.removeDuplicateVertices  = true;
                    m_settings.generateIndexBuffer      = true;
                    m_settings.weldIdenticalVertices    = true;
                    m_settings.removeUnusedVertices     = true;
                    m_settings.removeDegenerateTriangles= true;
                    m_settings.positionQuantization     = true;
                    m_settings.normalQuantization       = true;
                    m_settings.optimizeVertexCache      = true;
                    m_settings.optimizeVertexFetch      = true;
                    m_settings.quantizationBits         = 10;
                    break;
            }
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", desc);
        ImGui::PopStyleColor(3);
    };

    preset("⚡ Fast",     0, "Quick pass: remove dupes & degenerates");
    ImGui::SameLine(0, 6);
    preset("⚖ Balanced", 1, "All passes except quantization (recommended)");
    ImGui::SameLine(0, 6);
    preset("🔥 Maximum", 2, "Full pipeline including quantization");

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ─── Analytics Panel ───────────────────────────────────────────────────────────
void App::renderAnalyticsPanel(float panelW, float /*panelH*/) {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8, 8});

    float half = (panelW - 20.0f) * 0.5f;

    // Left: Stats table
    ImGui::BeginChild("##StatsChild", {half, 0}, false);
    Theme::SectionHeader("STATISTICS", "\xF0\x9F\x93\x8A");
    renderStatisticsTable();
    ImGui::EndChild();

    ImGui::SameLine(0, 8.0f);

    // Right: Processing log
    ImGui::BeginChild("##LogChild", {half, 0}, false);
    Theme::SectionHeader("PROCESSING LOG", "\xF0\x9F\x93\x9C");
    renderProcessingLog();
    ImGui::EndChild();

    ImGui::PopStyleVar();
}

// ─── Statistics Table ──────────────────────────────────────────────────────────
void App::renderStatisticsTable() {
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, Theme::Colors::BgDeep);
    ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, Theme::Colors::Border);
    ImGui::PushStyleColor(ImGuiCol_TableBorderLight,  {0.15f,0.18f,0.22f,1.0f});

    if (ImGui::BeginTable("##Stats", 3,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn("Metric",    ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Original",  ImGuiTableColumnFlags_WidthFixed, 90);
        ImGui::TableSetupColumn("Optimized", ImGuiTableColumnFlags_WidthFixed, 90);
        ImGui::TableHeadersRow();

        auto row = [&](const char* name, const std::string& orig, const std::string& opt,
                       bool highlight = false) {
            ImGui::TableNextRow();
            if (highlight && m_hasOptimized) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                    IM_COL32(0, 120, 60, 25));
            }
            ImGui::TableSetColumnIndex(0);
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextSecondary);
            ImGui::Text("%s", name);
            ImGui::PopStyleColor();
            ImGui::TableSetColumnIndex(1);
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextPrimary);
            ImGui::Text("%s", orig.c_str());
            ImGui::PopStyleColor();
            ImGui::TableSetColumnIndex(2);
            if (m_hasOptimized) {
                bool improved = (opt < orig);
                ImGui::PushStyleColor(ImGuiCol_Text,
                    highlight ? Theme::Colors::Green : Theme::Colors::TextPrimary);
                ImGui::Text("%s", opt.c_str());
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextDisabled);
                ImGui::Text("—");
                ImGui::PopStyleColor();
            }
        };

        bool has = m_hasOriginal;
        bool opt = m_hasOptimized;

        row("Vertices",
            has ? fmtNum(m_originalMesh.vertices.size()) : "—",
            opt ? fmtNum(m_optimizedMesh.vertices.size()) : "—", true);
        row("Triangles",
            has ? fmtNum(m_originalMesh.indices.size()/3) : "—",
            opt ? fmtNum(m_optimizedMesh.indices.size()/3) : "—");
        row("Indices",
            has ? fmtNum(m_originalMesh.indices.size()) : "—",
            opt ? fmtNum(m_optimizedMesh.indices.size()) : "—");
        row("Removed Dupes", "—",
            opt ? fmtNum(m_result.removedDuplicates) : "—");
        row("Vertex Memory",
            has ? fmtBytes(m_originalMesh.vertices.size()*sizeof(Vertex)) : "—",
            opt ? fmtBytes(m_optimizedMesh.vertices.size()*sizeof(Vertex)) : "—", true);
        row("Index Memory",
            has ? fmtBytes(m_originalMesh.indices.size()*4) : "—",
            opt ? fmtBytes(m_optimizedMesh.indices.size()*4) : "—");
        row("Total Memory",
            has ? fmtBytes(m_result.originalBytes > 0 ? m_result.originalBytes
                           : m_originalMesh.vertices.size()*sizeof(Vertex)) : "—",
            opt ? fmtBytes(m_result.optimizedBytes) : "—", true);

        // Compression metrics
        if (opt) {
            char ratioBuf[32];
            snprintf(ratioBuf, 32, "%.2fx", m_result.compressionRatio);
            row("Compression Ratio", "1.00x", ratioBuf, true);

            char saveBuf[32];
            snprintf(saveBuf, 32, "%.1f%%", m_result.percentageSaved);
            row("Savings", "0.0%", saveBuf, true);

            char timeBuf[32];
            snprintf(timeBuf, 32, "%.1f ms", m_result.processingTimeMs);
            row("Process Time", "—", timeBuf);

            char tpBuf[32];
            snprintf(tpBuf, 32, "%.2f MV/s", m_result.throughputMVs);
            row("Throughput", "—", tpBuf);
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleColor(3);
}

// ─── Processing Log ────────────────────────────────────────────────────────────
void App::renderProcessingLog() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::Colors::BgDeep);
    ImGui::BeginChild("##LogScroll", {0, 0}, true, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& entry : m_log) {
        ImVec4 col;
        const char* prefix;
        switch (entry.level) {
            case LogLevel::Success: col = Theme::Colors::Green;  prefix = "✔"; break;
            case LogLevel::Warning: col = Theme::Colors::Orange; prefix = "⚠"; break;
            case LogLevel::Error:   col = Theme::Colors::Red;    prefix = "✘"; break;
            default:                col = Theme::Colors::TextSecondary; prefix = "ℹ"; break;
        }
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::Text("[%.1fs] %s %s", entry.timestamp, prefix, entry.message.c_str());
        ImGui::PopStyleColor();
    }

    // Auto-scroll
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10.0f)
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ─── Viewport ──────────────────────────────────────────────────────────────────
void App::renderViewport(float x, float y, float w, float h) {
    // Render to FBO
    const MeshData& mesh = (m_hasOptimized && m_showOptimized)
        ? m_optimizedMesh : m_originalMesh;

    if ((int)w != m_viewW || (int)h != m_viewH) {
        m_viewW = (int)w; m_viewH = (int)h;
        m_renderer.resize(m_viewW, m_viewH);
    }

    m_renderer.render(mesh, m_camera, m_view, 0.0f);

    // Show in ImGui
    float fw = w, fh = h;
    ImVec2 uv0 = {0, 1}, uv1 = {1, 0}; // flip Y
    ImGui::Image((ImTextureID)(intptr_t)g_renderTexture,
                 {fw, fh}, uv0, uv1);

    // Viewport overlay (HUD)
    ImVec2 vPos = ImGui::GetItemRectMin();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Title bar overlay
    dl->AddRectFilled({vPos.x, vPos.y}, {vPos.x + w, vPos.y + 30},
                      IM_COL32(10, 12, 15, 180));
    dl->AddText({vPos.x + 10, vPos.y + 7}, IM_COL32(0, 200, 255, 220),
                "3D VIEWPORT");

    // FPS
    char fpsBuf[32];
    snprintf(fpsBuf, 32, "%.0f FPS", m_renderer.getFPS());
    dl->AddText({vPos.x + w - 60, vPos.y + 7}, IM_COL32(0, 200, 100, 200), fpsBuf);

    // Vertex count in corner
    if (m_hasOriginal) {
        char vcBuf[64];
        const MeshData& dm = (m_hasOptimized && m_showOptimized) ? m_optimizedMesh : m_originalMesh;
        snprintf(vcBuf, 64, "V: %s | T: %s",
            fmtNum(dm.vertices.size()).c_str(),
            fmtNum(dm.indices.size()/3).c_str());
        dl->AddText({vPos.x + 10, vPos.y + h - 22}, IM_COL32(120, 140, 160, 200), vcBuf);
    }

    // Mode indicator
    const char* modeStr = m_view.wireframe ? "[WIREFRAME]" : "[SOLID]";
    dl->AddText({vPos.x + w - 100, vPos.y + h - 22},
                IM_COL32(0, 180, 220, 180), modeStr);

    // Controls hint
    dl->AddText({vPos.x + w/2 - 80, vPos.y + h - 22},
                IM_COL32(80, 90, 100, 180),
                "Drag:Orbit  RMB:Pan  Scroll:Zoom");

    // Bottom toolbar inside viewport
    ImVec2 btPos = {vPos.x, vPos.y + h - 44};
    dl->AddRectFilled({btPos.x, btPos.y - 28}, {btPos.x + w, btPos.y},
                      IM_COL32(10, 12, 15, 160));

    // View control buttons (floating over viewport)
    ImGui::SetCursorPos({8, h - 52});
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {4, 0});

    auto viewBtn = [&](const char* lbl, bool& toggle) {
        if (toggle) {
            ImGui::PushStyleColor(ImGuiCol_Button, {0.00f, 0.40f, 0.55f, 0.90f});
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, {0.10f, 0.12f, 0.16f, 0.80f});
        }
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.00f, 0.55f, 0.75f, 0.95f});
        if (ImGui::Button(lbl, {0, 24})) toggle = !toggle;
        ImGui::PopStyleColor(2);
        ImGui::SameLine(0, 4);
    };

    viewBtn("Grid",    m_view.showGrid);
    viewBtn("Axes",    m_view.showAxes);
    viewBtn("BBox",    m_view.showBBox);
    viewBtn("Wire",    m_view.wireframe);
    viewBtn("Rotate",  m_view.rotating);

    // Fit button
    ImGui::PushStyleColor(ImGuiCol_Button, {0.10f, 0.12f, 0.16f, 0.80f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.00f, 0.55f, 0.75f, 0.95f});
    if (ImGui::Button("Fit", {0, 24}) && m_hasOriginal) {
        const MeshData& fm = m_showOptimized ? m_optimizedMesh : m_originalMesh;
        m_camera.target   = fm.center;
        m_camera.distance = fm.boundingRadius * 2.5f;
        m_camera.yaw      = 45.0f;
        m_camera.pitch    = 25.0f;
        m_camera.nearPlane = fm.boundingRadius * 0.001f;
        m_camera.farPlane  = fm.boundingRadius * 100.0f;
    }
    ImGui::PopStyleColor(2);

    ImGui::PopStyleVar(2);
}

// ─── Success Banner ────────────────────────────────────────────────────────────
void App::renderSuccessBanner() {
    float a = m_bannerAlpha;
    if (a < 0.01f) return;

    float bw = 420, bh = 110;
    float bx = (m_winW - bw) * 0.5f;
    float by = 80.0f;

    ImGui::SetNextWindowPos({bx, by});
    ImGui::SetNextWindowSize({bw, bh});
    ImGui::SetNextWindowBgAlpha(a * 0.95f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg,  {0.05f, 0.16f, 0.08f, a});
    ImGui::PushStyleColor(ImGuiCol_Border,    {0.20f, 0.85f, 0.35f, a});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

    ImGui::Begin("##SuccessBanner", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::SetCursorPosY(12.0f);

    float pulse = Theme::Pulse(2.0f);
    ImVec4 greenGlow = {0.20f + 0.1f*pulse, 0.85f + 0.1f*pulse, 0.35f, a};

    ImGui::PushStyleColor(ImGuiCol_Text, greenGlow);
    ImGui::SetWindowFontScale(1.6f);
    float tw = ImGui::CalcTextSize("\xE2\x9C\x94  Optimization Complete!").x;
    ImGui::SetCursorPosX((bw - tw) * 0.5f);
    ImGui::Text("\xE2\x9C\x94  Optimization Complete!");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, {0.6f, 1.0f, 0.7f, a});
    ImGui::SetWindowFontScale(1.3f);
    char saveBuf[64];
    snprintf(saveBuf, 64, "Storage Reduced by %.1f%%", m_result.percentageSaved);
    float tw2 = ImGui::CalcTextSize(saveBuf).x;
    ImGui::SetCursorPosX((bw - tw2) * 0.5f);
    ImGui::Text("%s", saveBuf);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, {0.5f, 0.8f, 0.6f, a * 0.8f});
    char infoBuf[128];
    snprintf(infoBuf, 128, "%s → %s  |  %.1f ms  |  %.2f MV/s",
        fmtBytes(m_result.originalBytes).c_str(),
        fmtBytes(m_result.optimizedBytes).c_str(),
        m_result.processingTimeMs,
        m_result.throughputMVs);
    float tw3 = ImGui::CalcTextSize(infoBuf).x;
    ImGui::SetCursorPosX((bw - tw3) * 0.5f);
    ImGui::Text("%s", infoBuf);
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

// ─── About Modal ───────────────────────────────────────────────────────────────
void App::renderAboutModal() {
    ImGui::SetNextWindowSize({520, 400}, ImGuiCond_Always);
    ImGui::SetNextWindowPos({(float)(m_winW - 520)/2, (float)(m_winH - 400)/2}, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::Colors::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_Border,   Theme::Colors::BorderGlow);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);

    ImGui::Begin("About poop3D Compiler", &m_showAbout,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    // Logo
    float p = Theme::Pulse(1.5f);
    ImVec4 logoCol = Theme::LerpColor({0.0f, 0.7f, 0.9f, 1.0f}, {0.2f, 1.0f, 1.0f, 1.0f}, p);
    ImGui::PushStyleColor(ImGuiCol_Text, logoCol);
    ImGui::SetWindowFontScale(1.8f);
    ImGui::Text("  \xF0\x9F\x92\xA9  poop3D Compiler");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextSecondary);
    ImGui::Text("Version 1.0.0  |  C++20 + OpenGL3 + Dear ImGui");
    ImGui::PopStyleColor();

    Theme::GlowSeparator();
    ImGui::Spacing();

    auto labeledText = [&](const char* label, const char* value, ImVec4 valColor) {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextSecondary);
        ImGui::Text("%s", label);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, valColor);
        ImGui::Text("%s", value);
        ImGui::PopStyleColor();
    };

    labeledText("Engineered & Developed by:", "  Hardik", Theme::Colors::Cyan);
    ImGui::Spacing();
    labeledText("Produced & Maintained by:", "  poop Organization, India \xF0\x9F\x87\xAE\xF0\x9F\x87\xB3",
                Theme::Colors::Green);
    ImGui::Spacing();
    Theme::GlowSeparator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextCyan);
    ImGui::Text("\xF0\x9F\x8E\xAF  Mission");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextSecondary);
    ImGui::TextWrapped(
        "Built to provide free, fast, privacy-friendly tools for 3D optimization "
        "without unnecessary complexity.\n\n"
        "poop3D Compiler processes everything locally — no cloud uploads, no data collection, "
        "no subscriptions. Your 3D assets stay yours.");
    ImGui::PopStyleColor();

    ImGui::Spacing();
    Theme::GlowSeparator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextSecondary);
    ImGui::Text("\xF0\x9F\x94\xA7  Technology Stack");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    struct Tech { const char* name; const char* desc; };
    Tech techs[] = {
        {"C++20",       "Core language with modern features"},
        {"Dear ImGui",  "Immediate-mode UI framework"},
        {"GLFW",        "Cross-platform window & input"},
        {"OpenGL 3.3",  "Hardware-accelerated rendering"},
        {"Assimp",      "3D file format import library"},
    };
    for (auto& t : techs) {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::Cyan);
        ImGui::Text("  ◆ %s", t.name);
        ImGui::PopStyleColor();
        ImGui::SameLine(140.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Colors::TextSecondary);
        ImGui::Text("%s", t.desc);
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::SetCursorPosX((520 - 100) * 0.5f);
    if (Theme::CyanButton("  Close  ", {100, 32})) m_showAbout = false;

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

// ─── Settings Modal ────────────────────────────────────────────────────────────
void App::renderSettingsModal() {
    ImGui::SetNextWindowSize({420, 320}, ImGuiCond_Always);
    ImGui::SetNextWindowPos({(float)(m_winW - 420)/2, (float)(m_winH - 320)/2}, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::Colors::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_Border,   Theme::Colors::BorderGlow);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);

    ImGui::Begin("Settings", &m_showSettings,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    Theme::SectionHeader("Export Settings", "\xF0\x9F\x92\xBE");

    ImGui::Text("Output path:");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##ExportPath", m_exportPath, sizeof(m_exportPath));

    const char* formats[] = {"OBJ (.obj)", "STL (.stl)", "PLY (.ply)"};
    ImGui::Text("Format:");
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("##Format", &m_selectedFormat, formats, 3);

    ImGui::Spacing();
    Theme::GlowSeparator();

    Theme::SectionHeader("Camera Settings", "\xF0\x9F\x8E\xA5");
    ImGui::SliderFloat("FOV",          &m_camera.fov,          30.0f, 120.0f, "%.0f deg");
    ImGui::SliderFloat("Rotate Speed", &m_view.rotateSpeed,    0.1f,  5.0f,  "%.1f");

    ImGui::Spacing();
    Theme::GlowSeparator();

    ImGui::SetCursorPosX((420 - 100) * 0.5f);
    if (Theme::CyanButton("  Close  ", {100, 32})) m_showSettings = false;

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

// ─── Actions ───────────────────────────────────────────────────────────────────
void App::generateMockMesh(int complexity) {
    if (m_state == AppState::Optimizing) return;
    m_state = AppState::Importing;
    addLog(LogLevel::Info, "Generating mock mesh (complexity " + std::to_string(complexity) + ")...");
    m_statusText  = "Generating procedural mesh...";
    m_statusColor = Theme::Colors::Orange;

    // Run in background
    MeshOptimizer optimizer;
    m_originalMesh    = MeshOptimizer::generateMockMesh(complexity);
    m_originalMesh.name = "MockMesh";
    m_hasOriginal     = true;
    m_hasOptimized    = false;
    m_showOptimized   = false;
    m_result          = {};
    m_state           = AppState::Idle;

    m_renderer.uploadMesh(m_originalMesh);
    m_camera.target   = m_originalMesh.center;
    m_camera.distance = m_originalMesh.boundingRadius * 2.5f;
    m_camera.nearPlane = m_originalMesh.boundingRadius * 0.001f;
    m_camera.farPlane  = m_originalMesh.boundingRadius * 100.0f;

    addLog(LogLevel::Success,
        "Mock mesh generated: " + fmtNum(m_originalMesh.vertices.size()) + " vertices, "
        + fmtNum(m_originalMesh.indices.size()/3) + " triangles");
    m_statusText  = "Mock mesh ready — " + fmtNum(m_originalMesh.vertices.size()) + " vertices";
    m_statusColor = Theme::Colors::Green;
}

void App::importModel(const std::string& path) {
    if (m_state == AppState::Optimizing) return;
    m_state = AppState::Importing;
    addLog(LogLevel::Info, "Importing: " + path);
    m_statusText  = "Importing " + path + "...";
    m_statusColor = Theme::Colors::Orange;

    MeshData mesh;
    bool ok = false;
    std::string err;

#ifdef HAVE_ASSIMP
    Assimp::Importer imp;
    unsigned flags = aiProcess_Triangulate | aiProcess_GenNormals |
                     aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords |
                     aiProcess_TransformUVCoords;
    const aiScene* scene = imp.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        err = imp.GetErrorString();
    } else {
        // Merge all meshes
        for (unsigned m = 0; m < scene->mNumMeshes; m++) {
            const aiMesh* am = scene->mMeshes[m];
            uint32_t base = (uint32_t)mesh.vertices.size();

            for (unsigned v = 0; v < am->mNumVertices; v++) {
                Vertex vert;
                vert.position = {am->mVertices[v].x, am->mVertices[v].y, am->mVertices[v].z};
                if (am->HasNormals())
                    vert.normal = {am->mNormals[v].x, am->mNormals[v].y, am->mNormals[v].z};
                if (am->HasTextureCoords(0))
                    vert.texcoord = {am->mTextureCoords[0][v].x, am->mTextureCoords[0][v].y};
                vert.color = {0.6f, 0.65f, 0.75f, 1.0f};
                mesh.vertices.push_back(vert);
            }
            for (unsigned f = 0; f < am->mNumFaces; f++) {
                const aiFace& face = am->mFaces[f];
                for (unsigned i = 0; i < face.mNumIndices; i++)
                    mesh.indices.push_back(base + face.mIndices[i]);
            }
        }
        mesh.name = path.substr(path.find_last_of("/\\") + 1);
        mesh.computeBounds();
        ok = true;
    }
#else
    // Built-in minimal OBJ parser
    if (path.size() >= 4 &&
        (path.substr(path.size()-4) == ".obj" || path.substr(path.size()-4) == ".OBJ"))
    {
        std::ifstream f(path);
        if (!f) { err = "Cannot open file: " + path; }
        else {
            std::vector<Vec3> positions, normals;
            std::vector<Vec2> uvs;
            std::string line;
            while (std::getline(f, line)) {
                if (line.size() < 2) continue;
                if (line[0]=='v' && line[1]==' ') {
                    Vec3 p; sscanf(line.c_str(), "v %f %f %f", &p.x, &p.y, &p.z);
                    positions.push_back(p);
                } else if (line[0]=='v' && line[1]=='n') {
                    Vec3 n; sscanf(line.c_str(), "vn %f %f %f", &n.x, &n.y, &n.z);
                    normals.push_back(n);
                } else if (line[0]=='v' && line[1]=='t') {
                    Vec2 uv; sscanf(line.c_str(), "vt %f %f", &uv.x, &uv.y);
                    uvs.push_back(uv);
                } else if (line[0]=='f') {
                    // Parse face: v/vt/vn
                    int vi[4]={0}, ti[4]={0}, ni[4]={0};
                    int cnt = 0;
                    const char* p = line.c_str() + 2;
                    while (*p && cnt < 4) {
                        sscanf(p, "%d/%d/%d", &vi[cnt], &ti[cnt], &ni[cnt]);
                        if (vi[cnt] == 0) sscanf(p, "%d//%d", &vi[cnt], &ni[cnt]);
                        if (vi[cnt] == 0) sscanf(p, "%d", &vi[cnt]);
                        cnt++;
                        while (*p && *p != ' ') p++;
                        while (*p == ' ') p++;
                    }
                    for (int t = 1; t+1 < cnt; t++) {
                        int tri[3] = {0, t, t+1};
                        for (int i : tri) {
                            Vertex vert = {};
                            int pi = vi[i]-1, tii = ti[i]-1, nii = ni[i]-1;
                            if (pi >= 0 && pi < (int)positions.size()) vert.position = positions[pi];
                            if (nii >= 0 && nii < (int)normals.size()) vert.normal = normals[nii];
                            if (tii >= 0 && tii < (int)uvs.size()) vert.texcoord = uvs[tii];
                            vert.color = {0.6f, 0.65f, 0.75f, 1.0f};
                            mesh.vertices.push_back(vert);
                        }
                    }
                }
            }
            mesh.name = path.substr(path.find_last_of("/\\") + 1);
            mesh.computeBounds();
            ok = !mesh.vertices.empty();
            if (!ok) err = "No geometry found in OBJ file";
        }
    } else {
        err = "Format not supported without Assimp. Build with -DHAVE_ASSIMP or use OBJ files.";
    }
#endif

    if (ok && !mesh.vertices.empty()) {
        m_originalMesh  = std::move(mesh);
        m_hasOriginal   = true;
        m_hasOptimized  = false;
        m_showOptimized = false;
        m_result        = {};
        m_state         = AppState::Idle;

        m_renderer.uploadMesh(m_originalMesh);
        m_camera.target    = m_originalMesh.center;
        m_camera.distance  = m_originalMesh.boundingRadius * 2.5f;
        m_camera.nearPlane = m_originalMesh.boundingRadius * 0.001f;
        m_camera.farPlane  = m_originalMesh.boundingRadius * 100.0f;

        addLog(LogLevel::Success,
            "Imported: " + m_originalMesh.name + " | "
            + fmtNum(m_originalMesh.vertices.size()) + " vertices");
        m_statusText  = "Model loaded: " + m_originalMesh.name;
        m_statusColor = Theme::Colors::Green;
    } else {
        m_state       = AppState::Error;
        m_statusText  = "Import failed: " + err;
        m_statusColor = Theme::Colors::Red;
        addLog(LogLevel::Error, "Import failed: " + err);
    }
}

void App::runOptimize() {
    if (!m_hasOriginal || m_state == AppState::Optimizing) return;

    m_cancelOptimize = false;
    m_state          = AppState::Optimizing;
    m_progress       = 0.0f;
    m_progressStage  = "Starting...";
    m_optimizeDone   = false;

    addLog(LogLevel::Info, "Starting optimization pipeline...");
    m_statusText  = "Optimizing mesh...";
    m_statusColor = Theme::Colors::Orange;

    MeshData inputCopy = m_originalMesh;
    OptimizeSettings settingsCopy = m_settings;

    m_workerThread = std::make_unique<std::thread>([this, inputCopy, settingsCopy]() mutable {
        MeshOptimizer opt;
        OptimizeResult res;

        MeshData output = opt.optimize(
            inputCopy, settingsCopy, res,
            [this](float p, const std::string& stage) {
                std::lock_guard<std::mutex> lk(m_progressMutex);
                m_threadProgress = p;
                m_threadStage    = stage;
            },
            &m_cancelOptimize
        );

        if (!m_cancelOptimize.load()) {
            m_optimizedMesh    = std::move(output);
            m_result           = res;
            m_optimizeSuccess  = res.success;
        } else {
            m_optimizeSuccess  = false;
            res.errorMsg       = "Cancelled";
        }
        m_optimizeDone = true;
    });
}

void App::exportMesh(const std::string& path) {
    const MeshData& mesh = m_hasOptimized ? m_optimizedMesh : m_originalMesh;
    if (mesh.vertices.empty()) {
        addLog(LogLevel::Warning, "Nothing to export");
        return;
    }

    // Export as OBJ
    std::string outPath = path;
    if (outPath.empty()) outPath = "output.obj";

    std::ofstream f(outPath);
    if (!f) {
        addLog(LogLevel::Error, "Cannot write to: " + outPath);
        return;
    }

    f << "# poop3D Compiler export\n";
    f << "# Vertices: " << mesh.vertices.size() << "\n";
    f << "# Triangles: " << mesh.indices.size()/3 << "\n\n";
    f << "o " << mesh.name << "\n\n";

    for (const auto& v : mesh.vertices)
        f << "v " << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
    f << "\n";
    for (const auto& v : mesh.vertices)
        f << "vn " << v.normal.x << " " << v.normal.y << " " << v.normal.z << "\n";
    f << "\n";
    for (const auto& v : mesh.vertices)
        f << "vt " << v.texcoord.x << " " << v.texcoord.y << "\n";
    f << "\n";

    if (!mesh.indices.empty()) {
        for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
            uint32_t a = mesh.indices[i]+1, b = mesh.indices[i+1]+1, c = mesh.indices[i+2]+1;
            f << "f " << a<<"/"<<a<<"/"<<a
              << " " << b<<"/"<<b<<"/"<<b
              << " " << c<<"/"<<c<<"/"<<c << "\n";
        }
    } else {
        for (size_t i = 0; i + 2 < mesh.vertices.size(); i += 3) {
            f << "f " << i+1 << " " << i+2 << " " << i+3 << "\n";
        }
    }

    f.close();
    addLog(LogLevel::Success, "Exported to: " + outPath);
    m_statusText  = "Exported: " + outPath;
    m_statusColor = Theme::Colors::Green;
}

void App::addLog(LogLevel level, const std::string& msg) {
    LogEntry e;
    e.level   = level;
    e.message = msg;
    e.timestamp = m_appTime;
    m_log.push_back(e);
    while (m_log.size() > MAX_LOG) m_log.pop_front();
}

void App::updateProgress(float p, const std::string& stage) {
    m_progress      = p;
    m_progressStage = stage;
}

// ─── Input Handling ────────────────────────────────────────────────────────────
void App::onMouseButton(int button, int action, int /*mods*/) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    if (button < 3)
        m_mouseDown[button] = (action != 0); // GLFW_RELEASE = 0
}

void App::onMouseMove(double x, double y) {
    ImGuiIO& io = ImGui::GetIO();

    // Check if mouse is in viewport
    float vx = m_viewX, vy = m_viewY;
    float vw = (float)m_viewW, vh = (float)m_viewH;
    m_mouseInView = (x >= vx && x <= vx + vw && y >= vy && y <= vy + vh);

    if (!io.WantCaptureMouse || m_mouseInView) {
        double dx = x - m_lastMouseX;
        double dy = y - m_lastMouseY;

        if (m_mouseDown[0] && m_mouseInView) {
            // Orbit
            m_camera.yaw   += (float)dx * 0.5f;
            m_camera.pitch += (float)dy * 0.5f;
            m_camera.pitch  = std::clamp(m_camera.pitch, -89.0f, 89.0f);
        }
        if (m_mouseDown[1] && m_mouseInView) {
            // Pan
            float s = m_camera.distance * 0.002f;
            Vec3 eye = m_camera.getPosition();
            Vec3 fwd = {m_camera.target.x - eye.x,
                        m_camera.target.y - eye.y,
                        m_camera.target.z - eye.z};
            float len = std::sqrt(fwd.x*fwd.x + fwd.y*fwd.y + fwd.z*fwd.z);
            if (len > 1e-8f) { fwd.x/=len; fwd.y/=len; fwd.z/=len; }
            Vec3 right = {fwd.z, 0, -fwd.x};
            float rlen = std::sqrt(right.x*right.x + right.z*right.z);
            if (rlen > 1e-8f) { right.x/=rlen; right.z/=rlen; }
            m_camera.target.x -= (float)dx * s * right.x;
            m_camera.target.z -= (float)dx * s * right.z;
            m_camera.target.y += (float)dy * s;
        }
    }

    m_lastMouseX = x;
    m_lastMouseY = y;
}

void App::onMouseScroll(double /*dx*/, double dy) {
    if (!m_mouseInView) return;
    m_camera.distance -= (float)dy * m_camera.distance * 0.1f;
    m_camera.distance  = std::clamp(m_camera.distance, 0.01f, 10000.0f);
}

void App::onKey(int key, int action, int /*mods*/) {
    if (action == 0) return; // GLFW_RELEASE
    if (key == 'W' || key == 'w') m_view.wireframe = !m_view.wireframe;
    if (key == 'G' || key == 'g') m_view.showGrid  = !m_view.showGrid;
    if (key == 'R' || key == 'r') m_view.rotating  = !m_view.rotating;
    if (key == 'F' || key == 'f') {
        if (m_hasOriginal) {
            const MeshData& fm = m_showOptimized ? m_optimizedMesh : m_originalMesh;
            m_camera.target   = fm.center;
            m_camera.distance = fm.boundingRadius * 2.5f;
            m_camera.yaw      = 45.0f;
            m_camera.pitch    = 25.0f;
        }
    }
}

void App::onDrop(int count, const char** paths) {
    if (count > 0) {
        importModel(std::string(paths[0]));
    }
}

void App::onResize(int w, int h) {
    m_winW = w; m_winH = h;
}
