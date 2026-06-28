#pragma once
#include "types.h"
#include "renderer.h"
#include "mesh_optimizer.h"
#include "ui_theme.h"
#include "imgui.h"
#include <string>
#include <vector>
#include <deque>
#include <atomic>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>

struct GLFWwindow;

class App {
public:
    App();
    ~App();

    bool init(GLFWwindow* window);
    void update(float dt);
    void render();
    void shutdown();

    // GLFW callbacks
    void onMouseButton(int button, int action, int mods);
    void onMouseMove(double x, double y);
    void onMouseScroll(double dx, double dy);
    void onKey(int key, int action, int mods);
    void onDrop(int count, const char** paths);
    void onResize(int w, int h);

private:
    // ─── UI Panels ───────────────────────────────────────────────────────────
    void renderTopToolbar();
    void renderOperationsPanel(float panelW, float panelH);
    void renderAnalyticsPanel(float panelW, float panelH);
    void renderViewport(float x, float y, float w, float h);
    void renderStatusBar(float h);
    void renderAboutModal();
    void renderSettingsModal();
    void renderSuccessBanner();
    void renderImportDropzone();

    // ─── Sub-sections ────────────────────────────────────────────────────────
    void renderOptimizationSettings();
    void renderCompressionPresets();
    void renderStatisticsTable();
    void renderProcessingLog();

    // ─── Actions ─────────────────────────────────────────────────────────────
    void importModel(const std::string& path);
    void generateMockMesh(int complexity);
    void runOptimize();
    void exportMesh(const std::string& path);

    void addLog(LogLevel level, const std::string& msg);
    void updateProgress(float p, const std::string& stage);

    // ─── State ───────────────────────────────────────────────────────────────
    GLFWwindow*     m_window     = nullptr;
    AppState        m_state      = AppState::Idle;
    bool            m_showAbout  = false;
    bool            m_showSettings = false;

    // Mesh data
    MeshData        m_originalMesh;
    MeshData        m_optimizedMesh;
    bool            m_hasOriginal  = false;
    bool            m_hasOptimized = false;
    bool            m_showOptimized = false;

    // Settings & results
    OptimizeSettings m_settings;
    OptimizeResult   m_result;

    // Progress
    float           m_progress    = 0.0f;
    std::string     m_progressStage;

    // Optimization thread
    std::unique_ptr<std::thread> m_workerThread;
    std::atomic<bool>            m_cancelOptimize{false};
    std::mutex                   m_progressMutex;
    float                        m_threadProgress = 0.0f;
    std::string                  m_threadStage;
    std::atomic<bool>            m_optimizeDone{false};
    bool                         m_optimizeSuccess = false;

    // Log
    std::deque<LogEntry>         m_log;
    static constexpr int         MAX_LOG = 200;

    // Camera & view
    Camera                       m_camera;
    ViewSettings                 m_view;
    float                        m_viewRotation = 0.0f;

    // Renderer
    Renderer                     m_renderer;
    int                          m_viewW = 800, m_viewH = 600;
    float                        m_viewX = 0,  m_viewY = 0;

    // Mouse state
    bool    m_mouseDown[3]  = {};
    double  m_lastMouseX    = 0, m_lastMouseY = 0;
    bool    m_mouseInView   = false;

    // Animations
    float   m_successTimer  = 0.0f;
    float   m_fadeInTimer   = 0.0f;
    float   m_bannerAlpha   = 0.0f;
    float   m_panelFade     = 0.0f;

    // App size
    int     m_winW = 1400, m_winH = 900;

    // Status
    std::string m_statusText = "Ready — Import or generate a 3D model to begin";
    ImVec4      m_statusColor;

    // Export
    char   m_exportPath[512] = "output.obj";

    // Misc
    float  m_appTime = 0.0f;
    int    m_selectedFormat = 0;

    // Mock mesh gen flag
    bool   m_generatingMock = false;
};
