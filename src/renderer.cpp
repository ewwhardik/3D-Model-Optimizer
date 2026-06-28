#include "renderer.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <GLFW/glfw3.h>

GLuint g_renderTexture = 0;

// ─── Shader Sources ────────────────────────────────────────────────────────────
static const char* MESH_VERT = R"GLSL(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
layout(location=3) in vec4 aColor;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform vec3 uLightDir;
uniform int  uWireframe;

out vec3 vNormal;
out vec3 vWorldPos;
out vec4 vColor;
out vec2 vUV;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal   = mat3(uModel) * aNormal;
    vColor    = aColor;
    vUV       = aUV;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)GLSL";

static const char* MESH_FRAG = R"GLSL(
#version 330 core
in vec3 vNormal;
in vec3 vWorldPos;
in vec4 vColor;
in vec2 vUV;

uniform vec3 uCamPos;
uniform vec3 uLightDir;
uniform int  uWireframe;
uniform vec3 uBaseColor;

out vec4 FragColor;

void main() {
    if (uWireframe == 1) {
        FragColor = vec4(0.0, 0.85, 1.0, 1.0);
        return;
    }

    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uCamPos - vWorldPos);
    vec3 H = normalize(L + V);

    // Ambient
    vec3 ambient = vec3(0.12, 0.14, 0.18);

    // Diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * vec3(0.75, 0.78, 0.85);

    // Fill light
    float fillDiff = max(dot(N, vec3(-0.5, 0.3, -0.5)), 0.0);
    vec3  fill = fillDiff * vec3(0.1, 0.15, 0.25);

    // Specular
    float spec = pow(max(dot(N, H), 0.0), 64.0) * 0.6;
    vec3 specular = spec * vec3(0.8, 0.9, 1.0);

    // Rim light
    float rim = 1.0 - max(dot(N, V), 0.0);
    rim = pow(rim, 3.0) * 0.35;
    vec3 rimColor = rim * vec3(0.0, 0.7, 1.0);

    // Base color
    vec3 matColor = uBaseColor;
    if (vColor.a > 0.01) matColor = vColor.rgb;

    vec3 final = (ambient + diffuse + fill) * matColor + specular + rimColor;

    // Tone mapping
    final = final / (final + vec3(1.0));
    final = pow(final, vec3(1.0/2.2));

    FragColor = vec4(final, 1.0);
}
)GLSL";

static const char* FLAT_VERT = R"GLSL(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec4 aColor;

uniform mat4 uMVP;
out vec4 vColor;

void main() {
    vColor = aColor;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)GLSL";

static const char* FLAT_FRAG = R"GLSL(
#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main() {
    FragColor = vColor;
}
)GLSL";

// ─── Math helpers ──────────────────────────────────────────────────────────────
Mat4 Renderer::perspective(float fovDeg, float aspect, float nearZ, float farZ) const {
    Mat4 m = {};
    float f = 1.0f / std::tan(fovDeg * 3.14159f / 360.0f);
    m.m[0]  = f / aspect;
    m.m[5]  = f;
    m.m[10] = (farZ + nearZ) / (nearZ - farZ);
    m.m[11] = -1.0f;
    m.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
    return m;
}

Mat4 Renderer::lookAt(Vec3 eye, Vec3 center, Vec3 up) const {
    Vec3 f = {center.x-eye.x, center.y-eye.y, center.z-eye.z};
    float flen = std::sqrt(f.x*f.x+f.y*f.y+f.z*f.z);
    if (flen > 1e-8f) { f.x/=flen; f.y/=flen; f.z/=flen; }

    Vec3 s = {f.y*up.z-f.z*up.y, f.z*up.x-f.x*up.z, f.x*up.y-f.y*up.x};
    float slen = std::sqrt(s.x*s.x+s.y*s.y+s.z*s.z);
    if (slen > 1e-8f) { s.x/=slen; s.y/=slen; s.z/=slen; }

    Vec3 u = {s.y*f.z-s.z*f.y, s.z*f.x-s.x*f.z, s.x*f.y-s.y*f.x};

    Mat4 m = {};
    m.m[0]=s.x; m.m[4]=s.y; m.m[8]=s.z;
    m.m[1]=u.x; m.m[5]=u.y; m.m[9]=u.z;
    m.m[2]=-f.x; m.m[6]=-f.y; m.m[10]=-f.z;
    m.m[12]=-(s.x*eye.x+s.y*eye.y+s.z*eye.z);
    m.m[13]=-(u.x*eye.x+u.y*eye.y+u.z*eye.z);
    m.m[14]=(f.x*eye.x+f.y*eye.y+f.z*eye.z);
    m.m[15]=1.0f;
    return m;
}

Mat4 Renderer::multiply(const Mat4& a, const Mat4& b) const {
    Mat4 r = {};
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            for (int k = 0; k < 4; k++)
                r.m[col*4+row] += a.m[k*4+row] * b.m[col*4+k];
    return r;
}

// ─── Compile Shader ────────────────────────────────────────────────────────────
GLuint Renderer::compileShader(const char* vert, const char* frag) {
    auto compile = [](GLenum type, const char* src) -> GLuint {
        GLuint sh = glCreateShader(type);
        glShaderSource(sh, 1, &src, nullptr);
        glCompileShader(sh);
        GLint ok = 0; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char buf[1024]; glGetShaderInfoLog(sh, 1024, nullptr, buf);
            fprintf(stderr, "Shader error: %s\n", buf);
        }
        return sh;
    };
    GLuint vs = compile(GL_VERTEX_SHADER, vert);
    GLuint fs = compile(GL_FRAGMENT_SHADER, frag);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs); glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs); glDeleteShader(fs);
    return prog;
}

// ─── Init ──────────────────────────────────────────────────────────────────────
Renderer::Renderer()  = default;
Renderer::~Renderer() { shutdown(); }

bool Renderer::init(int w, int h) {
    m_width = w; m_height = h;
    m_progMesh = compileShader(MESH_VERT, MESH_FRAG);
    m_progFlat = compileShader(FLAT_VERT, FLAT_FRAG);
    m_progGrid = m_progFlat; // reuse flat shader for grid

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    buildGridGeometry();
    buildAxisGeometry();

    // Create FBO for offscreen render
    glGenFramebuffers(1, &m_fbo);
    glGenTextures(1, &m_colorTex);
    glGenRenderbuffers(1, &m_depthRbo);
    resize(w, h);

    g_renderTexture = m_colorTex;
    return true;
}

void Renderer::resize(int w, int h) {
    m_width = w; m_height = h;
    if (m_fbo == 0) return;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::uploadMesh(const MeshData& mesh) {
    m_vertCount  = mesh.vertices.size();
    m_idxCount   = mesh.indices.size();
    m_hasIndex   = !mesh.indices.empty();
    m_boundsMin  = mesh.boundsMin;
    m_boundsMax  = mesh.boundsMax;

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertCount * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

    if (m_hasIndex) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_idxCount * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);
    }

    // Attrib: position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    // Attrib: normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // Attrib: texcoord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
    // Attrib: color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glBindVertexArray(0);
}

// ─── Grid ──────────────────────────────────────────────────────────────────────
void Renderer::buildGridGeometry() {
    struct GV { Vec3 pos; Vec4 col; };
    std::vector<GV> lines;
    int   N    = 20;
    float step = 1.0f;
    float fade = 0.28f;
    float bold = 0.45f;

    for (int i = -N; i <= N; i++) {
        float f = (i == 0) ? bold : fade;
        float x = i * step;
        // Along Z
        lines.push_back({{x, 0, -(float)N}, {f,f,f,1}});
        lines.push_back({{x, 0,  (float)N}, {f,f,f,1}});
        // Along X
        lines.push_back({{-(float)N, 0, x}, {f,f,f,1}});
        lines.push_back({{ (float)N, 0, x}, {f,f,f,1}});
    }

    glGenVertexArrays(1, &m_gridVao);
    glGenBuffers(1, &m_gridVbo);
    glBindVertexArray(m_gridVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVbo);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(GV), lines.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GV), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GV), (void*)sizeof(Vec3));
    glBindVertexArray(0);
    // Grid line count is computed from N in render()
    (void)lines.size();
}

void Renderer::buildAxisGeometry() {
    struct AV { Vec3 pos; Vec4 col; };
    float L = 2.0f;
    AV axes[] = {
        {{0,0,0},{1,0.2f,0.2f,1}}, {{L,0,0},{1,0.2f,0.2f,1}}, // X red
        {{0,0,0},{0.2f,1,0.2f,1}}, {{0,L,0},{0.2f,1,0.2f,1}}, // Y green
        {{0,0,0},{0.2f,0.5f,1,1}}, {{0,0,L},{0.2f,0.5f,1,1}}, // Z blue
    };
    glGenVertexArrays(1, &m_axisVao);
    glGenBuffers(1, &m_axisVbo);
    glBindVertexArray(m_axisVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_axisVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AV), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(AV), (void*)sizeof(Vec3));
    glBindVertexArray(0);
}

// ─── Main Render ───────────────────────────────────────────────────────────────
void Renderer::render(const MeshData& mesh, const Camera& cam, const ViewSettings& view, float /*fps*/) {
    // FPS calc
    double now = glfwGetTime();
    m_frameCount++;
    if (now - m_lastTime >= 0.5) {
        m_fps = (float)m_frameCount / (float)(now - m_lastTime);
        m_frameCount = 0;
        m_lastTime = now;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);

    // Background gradient via clear
    glClearColor(0.07f, 0.09f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Matrices
    float aspect = m_height > 0 ? (float)m_width / m_height : 1.0f;
    Vec3  eye    = cam.getPosition();
    Mat4  proj   = perspective(cam.fov, aspect, cam.nearPlane, cam.farPlane);
    Mat4  view4  = lookAt(eye, cam.target, {0,1,0});
    Mat4  vp     = multiply(proj, view4);

    // Identity model matrix
    Mat4 model = {};
    model.m[0] = model.m[5] = model.m[10] = model.m[15] = 1.0f;
    Mat4 mvp = multiply(vp, model);

    // Grid
    if (view.showGrid && m_gridVao) {
        glUseProgram(m_progFlat);
        glUniformMatrix4fv(glGetUniformLocation(m_progFlat, "uMVP"), 1, GL_FALSE, mvp.m);
        glBindVertexArray(m_gridVao);
        glLineWidth(1.0f);
        int N = 20;
        // (2*N+1) lines in X dir + (2*N+1) lines in Z dir, 2 verts each
        int gridLineVerts = (N*2+1) * 2 * 2; // both directions
        glDrawArrays(GL_LINES, 0, gridLineVerts);
        glBindVertexArray(0);
    }

    // Axes
    if (view.showAxes && m_axisVao) {
        glUseProgram(m_progFlat);
        glUniformMatrix4fv(glGetUniformLocation(m_progFlat, "uMVP"), 1, GL_FALSE, mvp.m);
        glBindVertexArray(m_axisVao);
        glLineWidth(2.5f);
        glDrawArrays(GL_LINES, 0, 6);
        glBindVertexArray(0);
    }

    // Mesh
    if (m_vertCount > 0 && m_vao) {
        glUseProgram(m_progMesh);
        glUniformMatrix4fv(glGetUniformLocation(m_progMesh, "uMVP"),   1, GL_FALSE, mvp.m);
        glUniformMatrix4fv(glGetUniformLocation(m_progMesh, "uModel"), 1, GL_FALSE, model.m);
        glUniform3f(glGetUniformLocation(m_progMesh, "uLightDir"), 0.6f, 1.0f, 0.7f);
        glUniform3f(glGetUniformLocation(m_progMesh, "uCamPos"), eye.x, eye.y, eye.z);
        glUniform3f(glGetUniformLocation(m_progMesh, "uBaseColor"), 0.45f, 0.52f, 0.65f);
        glUniform1i(glGetUniformLocation(m_progMesh, "uWireframe"), view.wireframe ? 1 : 0);

        glBindVertexArray(m_vao);

        if (view.wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(1.0f);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        if (m_hasIndex)
            glDrawElements(GL_TRIANGLES, (GLsizei)m_idxCount, GL_UNSIGNED_INT, nullptr);
        else
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_vertCount);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(0);

        // Bounding box
        if (view.showBBox) renderBBox(mvp);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::renderBBox(const Mat4& mvp) {
    Vec3 mn = m_boundsMin, mx = m_boundsMax;
    struct BV { Vec3 pos; Vec4 col; };
    Vec4 col = {0.0f, 0.85f, 1.0f, 0.8f};
    // 12 edges of box
    BV lines[] = {
        {{mn.x,mn.y,mn.z},col},{{mx.x,mn.y,mn.z},col},
        {{mx.x,mn.y,mn.z},col},{{mx.x,mn.y,mx.z},col},
        {{mx.x,mn.y,mx.z},col},{{mn.x,mn.y,mx.z},col},
        {{mn.x,mn.y,mx.z},col},{{mn.x,mn.y,mn.z},col},
        {{mn.x,mx.y,mn.z},col},{{mx.x,mx.y,mn.z},col},
        {{mx.x,mx.y,mn.z},col},{{mx.x,mx.y,mx.z},col},
        {{mx.x,mx.y,mx.z},col},{{mn.x,mx.y,mx.z},col},
        {{mn.x,mx.y,mx.z},col},{{mn.x,mx.y,mn.z},col},
        {{mn.x,mn.y,mn.z},col},{{mn.x,mx.y,mn.z},col},
        {{mx.x,mn.y,mn.z},col},{{mx.x,mx.y,mn.z},col},
        {{mx.x,mn.y,mx.z},col},{{mx.x,mx.y,mx.z},col},
        {{mn.x,mn.y,mx.z},col},{{mn.x,mx.y,mx.z},col},
    };

    GLuint vao2, vbo2;
    glGenVertexArrays(1, &vao2);
    glGenBuffers(1, &vbo2);
    glBindVertexArray(vao2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BV), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(BV), (void*)sizeof(Vec3));

    glUseProgram(m_progFlat);
    glUniformMatrix4fv(glGetUniformLocation(m_progFlat, "uMVP"), 1, GL_FALSE, mvp.m);
    glLineWidth(1.5f);
    glDrawArrays(GL_LINES, 0, 24);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao2);
    glDeleteBuffers(1, &vbo2);
}

void Renderer::shutdown() {
    if (m_vao)    { glDeleteVertexArrays(1, &m_vao);    m_vao = 0; }
    if (m_vbo)    { glDeleteBuffers(1, &m_vbo);          m_vbo = 0; }
    if (m_ebo)    { glDeleteBuffers(1, &m_ebo);          m_ebo = 0; }
    if (m_gridVao){ glDeleteVertexArrays(1,&m_gridVao); m_gridVao=0;}
    if (m_gridVbo){ glDeleteBuffers(1,&m_gridVbo);       m_gridVbo=0;}
    if (m_axisVao){ glDeleteVertexArrays(1,&m_axisVao); m_axisVao=0;}
    if (m_axisVbo){ glDeleteBuffers(1,&m_axisVbo);       m_axisVbo=0;}
    if (m_fbo)    { glDeleteFramebuffers(1,&m_fbo);      m_fbo=0; }
    if (m_colorTex){glDeleteTextures(1,&m_colorTex);    m_colorTex=0;}
    if (m_depthRbo){glDeleteRenderbuffers(1,&m_depthRbo);m_depthRbo=0;}
    if (m_progMesh){ glDeleteProgram(m_progMesh); m_progMesh=0; }
    if (m_progFlat){ glDeleteProgram(m_progFlat); m_progFlat=0; }
}
