#pragma once
#include "types.h"
#include "glad/glad.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(int width, int height);
    void resize(int width, int height);
    void render(const MeshData& mesh, const Camera& cam, const ViewSettings& view, float fps);
    void uploadMesh(const MeshData& mesh);
    void shutdown();

    bool  hasValidMesh() const { return m_vertCount > 0; }
    float getFPS()       const { return m_fps; }

private:
    // OpenGL objects
    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
    GLuint m_gridVao = 0, m_gridVbo = 0;
    GLuint m_axisVao = 0, m_axisVbo = 0;
    GLuint m_progMesh   = 0;
    GLuint m_progFlat   = 0;
    GLuint m_progGrid   = 0;
    GLuint m_fbo = 0, m_colorTex = 0, m_depthRbo = 0;

    int    m_width = 800, m_height = 600;
    size_t m_vertCount = 0;
    size_t m_idxCount  = 0;
    bool   m_hasIndex  = false;
    float  m_fps       = 0.0f;
    double m_lastTime  = 0.0;
    int    m_frameCount = 0;

    Vec3   m_boundsMin = {}, m_boundsMax = {};

    void buildGridGeometry();
    void buildAxisGeometry();
    void renderGrid(const Mat4& vp);
    void renderAxes(const Mat4& vp);
    void renderBBox(const Mat4& vp);

    GLuint compileShader(const char* vert, const char* frag);
    Mat4   perspective(float fov, float aspect, float near, float far) const;
    Mat4   lookAt(Vec3 eye, Vec3 center, Vec3 up) const;
    Mat4   multiply(const Mat4& a, const Mat4& b) const;
};

// Expose texture for ImGui
extern GLuint g_renderTexture;
