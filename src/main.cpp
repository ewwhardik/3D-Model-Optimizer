#include "app.h"
#include "glad/glad.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// ─── Global App ───────────────────────────────────────────────────────────────
static App* g_app = nullptr;

// ─── GLFW Callbacks ──────────────────────────────────────────────────────────
static void cb_error(int code, const char* desc) {
    fprintf(stderr, "[GLFW Error %d]: %s\n", code, desc);
}

static void cb_mouseButton(GLFWwindow* w, int btn, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(w, btn, action, mods);
    if (g_app) g_app->onMouseButton(btn, action, mods);
}

static void cb_cursorPos(GLFWwindow* /*w*/, double x, double y) {
    if (g_app) g_app->onMouseMove(x, y);
}

static void cb_scroll(GLFWwindow* w, double dx, double dy) {
    ImGui_ImplGlfw_ScrollCallback(w, dx, dy);
    if (g_app) g_app->onMouseScroll(dx, dy);
}

static void cb_key(GLFWwindow* w, int key, int scan, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(w, key, scan, action, mods);
    if (g_app && !ImGui::GetIO().WantCaptureKeyboard)
        g_app->onKey(key, action, mods);
}

static void cb_char(GLFWwindow* w, unsigned int c) {
    ImGui_ImplGlfw_CharCallback(w, c);
}

static void cb_drop(GLFWwindow* /*w*/, int count, const char** paths) {
    if (g_app) g_app->onDrop(count, paths);
}

static void cb_framebufferSize(GLFWwindow* /*w*/, int width, int height) {
    glViewport(0, 0, width, height);
    if (g_app) g_app->onResize(width, height);
}

// ─── Font loading ─────────────────────────────────────────────────────────────
static void loadFonts(ImGuiIO& io) {
    io.Fonts->Clear();

    // Embed a compact default font - we use ImGui's built-in
    ImFontConfig cfg;
    cfg.OversampleH = 3;
    cfg.OversampleV = 3;
    cfg.PixelSnapH  = false;

    // Primary font - slightly larger than default
    io.Fonts->AddFontDefault(&cfg);

    // Add icons from default font at bigger size
    ImFontConfig cfgBig;
    cfgBig.MergeMode  = false;
    cfgBig.SizePixels = 13.0f;
    // Just use default - a real app would load a .ttf here
    io.Fonts->Build();
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main(int /*argc*/, char** /*argv*/) {
    glfwSetErrorCallback(cb_error);

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    // OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Get monitor for centering
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    int winW = 1440, winH = 900;
    if (mode) {
        winW = (int)(mode->width  * 0.88f);
        winH = (int)(mode->height * 0.88f);
        winW = winW < 1200 ? 1200 : winW;
        winH = winH < 800  ? 800  : winH;
    }

    GLFWwindow* window = glfwCreateWindow(
        winW, winH,
        "\xF0\x9F\x92\xA9  poop3D Compiler v1.0  —  3D Model Optimizer",
        nullptr, nullptr
    );

    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }

    // Center window
    if (mode) {
        glfwSetWindowPos(window,
            (mode->width  - winW) / 2,
            (mode->height - winH) / 2);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    printf("OpenGL %s\n", glGetString(GL_VERSION));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));

    // Register callbacks
    glfwSetMouseButtonCallback(window,    cb_mouseButton);
    glfwSetCursorPosCallback(window,      cb_cursorPos);
    glfwSetScrollCallback(window,         cb_scroll);
    glfwSetKeyCallback(window,            cb_key);
    glfwSetCharCallback(window,           cb_char);
    glfwSetDropCallback(window,           cb_drop);
    glfwSetFramebufferSizeCallback(window,cb_framebufferSize);

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename  = "poop3d_layout.ini";

    loadFonts(io);

    ImGui_ImplGlfw_InitForOpenGL(window, false); // false = don't install callbacks (we do it manually)
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Instantiate & init app
    App app;
    g_app = &app;
    if (!app.init(window)) {
        fprintf(stderr, "App init failed\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    double lastTime = glfwGetTime();

    // ─── Main Loop ────────────────────────────────────────────────────────────
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double now = glfwGetTime();
        float  dt  = (float)(now - lastTime);
        if (dt > 0.1f) dt = 0.1f; // cap
        lastTime = now;

        // Update app logic
        app.update(dt);

        // Begin ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render app UI
        app.render();

        // Render ImGui
        ImGui::Render();

        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);
        glClearColor(0.06f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    app.shutdown();
    g_app = nullptr;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
