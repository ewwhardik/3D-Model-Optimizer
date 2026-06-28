#pragma once
#include <string>
#include <vector>
#include <array>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <optional>
#include <deque>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cstring>

// ─── Math Types ────────────────────────────────────────────────────────────────
struct Vec2 { float x = 0, y = 0; };
struct Vec3 { float x = 0, y = 0, z = 0; };
struct Vec4 { float x = 0, y = 0, z = 0, w = 1; };
struct Mat4 { float m[16] = {}; };

// ─── Mesh Data ─────────────────────────────────────────────────────────────────
struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texcoord;
    Vec4 color;
};

struct MeshData {
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
    std::string           name;
    Vec3                  boundsMin, boundsMax;
    Vec3                  center;
    float                 boundingRadius = 1.0f;

    void computeBounds() {
        if (vertices.empty()) return;
        boundsMin = boundsMax = vertices[0].position;
        for (auto& v : vertices) {
            boundsMin.x = std::min(boundsMin.x, v.position.x);
            boundsMin.y = std::min(boundsMin.y, v.position.y);
            boundsMin.z = std::min(boundsMin.z, v.position.z);
            boundsMax.x = std::max(boundsMax.x, v.position.x);
            boundsMax.y = std::max(boundsMax.y, v.position.y);
            boundsMax.z = std::max(boundsMax.z, v.position.z);
        }
        center.x = (boundsMin.x + boundsMax.x) * 0.5f;
        center.y = (boundsMin.y + boundsMax.y) * 0.5f;
        center.z = (boundsMin.z + boundsMax.z) * 0.5f;
        float dx = boundsMax.x - boundsMin.x;
        float dy = boundsMax.y - boundsMin.y;
        float dz = boundsMax.z - boundsMin.z;
        boundingRadius = std::sqrt(dx*dx + dy*dy + dz*dz) * 0.5f;
    }
};

// ─── Optimization Settings ─────────────────────────────────────────────────────
struct OptimizeSettings {
    bool removeDuplicateVertices = true;
    bool generateIndexBuffer     = true;
    bool weldIdenticalVertices   = true;
    bool removeUnusedVertices    = true;
    bool removeDegenerateTriangles = true;
    bool positionQuantization    = false;
    bool normalQuantization      = false;
    bool optimizeVertexCache     = true;
    bool optimizeVertexFetch     = true;

    int  quantizationBits        = 10;
    int  preset                  = 1; // 0=Fast, 1=Balanced, 2=Maximum
    int  mockComplexity          = 1; // 0=50K, 1=250K, 2=1M vertices
};

// ─── Optimization Results ──────────────────────────────────────────────────────
struct OptimizeResult {
    size_t originalVertices   = 0;
    size_t optimizedVertices  = 0;
    size_t removedDuplicates  = 0;
    size_t triangleCount      = 0;
    size_t indexCount         = 0;
    size_t originalBytes      = 0;
    size_t optimizedBytes     = 0;
    double compressionRatio   = 1.0;
    double percentageSaved    = 0.0;
    double processingTimeMs   = 0.0;
    double throughputMVs      = 0.0; // Mega-vertices/sec
    bool   success            = false;
    std::string errorMsg;
};

// ─── Log Entry ─────────────────────────────────────────────────────────────────
enum class LogLevel { Info, Success, Warning, Error };

struct LogEntry {
    LogLevel    level;
    std::string message;
    float       timestamp;
};

// ─── App State ─────────────────────────────────────────────────────────────────
enum class AppState {
    Idle,
    Importing,
    Optimizing,
    Done,
    Error
};

// ─── Camera ────────────────────────────────────────────────────────────────────
struct Camera {
    float yaw        = 45.0f;
    float pitch      = 25.0f;
    float distance   = 3.0f;
    Vec3  target     = {0, 0, 0};
    float fov        = 60.0f;
    float nearPlane  = 0.001f;
    float farPlane   = 10000.0f;

    Vec3 getPosition() const {
        float radY = yaw   * 3.14159f / 180.0f;
        float radP = pitch * 3.14159f / 180.0f;
        return {
            target.x + distance * std::cos(radP) * std::sin(radY),
            target.y + distance * std::sin(radP),
            target.z + distance * std::cos(radP) * std::cos(radY)
        };
    }
};

// ─── View Settings ─────────────────────────────────────────────────────────────
struct ViewSettings {
    bool  wireframe    = false;
    bool  showGrid     = true;
    bool  showAxes     = true;
    bool  showBBox     = false;
    bool  showNormals  = false;
    bool  rotating     = false;
    float rotateSpeed  = 0.3f;
};
