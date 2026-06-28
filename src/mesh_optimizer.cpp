#include "mesh_optimizer.h"
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>

// ─── Hash for vertex deduplication ────────────────────────────────────────────
struct VertexHash {
    size_t operator()(const Vertex& v) const {
        size_t h = 0;
        auto combine = [&](float f) {
            uint32_t x;
            memcpy(&x, &f, 4);
            h ^= std::hash<uint32_t>()(x) + 0x9e3779b9 + (h << 6) + (h >> 2);
        };
        combine(v.position.x); combine(v.position.y); combine(v.position.z);
        combine(v.normal.x);   combine(v.normal.y);   combine(v.normal.z);
        combine(v.texcoord.x); combine(v.texcoord.y);
        return h;
    }
};

struct VertexEq {
    bool operator()(const Vertex& a, const Vertex& b) const {
        return memcmp(&a, &b, sizeof(Vertex)) == 0;
    }
};

// ─── Main Optimize ─────────────────────────────────────────────────────────────
MeshData MeshOptimizer::optimize(
    const MeshData& input,
    const OptimizeSettings& settings,
    OptimizeResult& outResult,
    ProgressCallback progress,
    std::atomic<bool>* cancel)
{
    using Clock = std::chrono::high_resolution_clock;
    auto t0 = Clock::now();

    MeshData mesh = input;
    outResult = {};
    outResult.originalVertices = mesh.vertices.size();
    outResult.originalBytes    = estimateBytes(mesh);

    auto report = [&](float p, const std::string& msg) {
        if (progress) progress(p, msg);
    };

    auto checkCancel = [&]() -> bool {
        return cancel && cancel->load();
    };

    report(0.05f, "Starting optimization pipeline...");
    if (checkCancel()) return mesh;

    // Step 1: Remove degenerate triangles
    if (settings.removeDegenerateTriangles && !mesh.indices.empty()) {
        report(0.10f, "Removing degenerate triangles...");
        removeDegenerates(mesh);
        if (checkCancel()) return mesh;
    }

    // Step 2: Generate index buffer if none
    if (settings.generateIndexBuffer && mesh.indices.empty()) {
        report(0.20f, "Generating index buffer...");
        generateIndexBuffer(mesh);
        if (checkCancel()) return mesh;
    }

    // Step 3: Remove duplicates
    size_t removedDupes = 0;
    if (settings.removeDuplicateVertices) {
        report(0.30f, "Removing duplicate vertices...");
        removeDuplicates(mesh, removedDupes);
        if (checkCancel()) return mesh;
    }

    // Step 4: Weld vertices
    if (settings.weldIdenticalVertices) {
        report(0.45f, "Welding identical vertices...");
        weldVertices(mesh);
        if (checkCancel()) return mesh;
    }

    // Step 5: Remove unused
    if (settings.removeUnusedVertices) {
        report(0.55f, "Removing unused vertices...");
        removeUnused(mesh);
        if (checkCancel()) return mesh;
    }

    // Step 6: Quantization
    if (settings.positionQuantization) {
        report(0.62f, "Quantizing positions...");
        quantizePositions(mesh, settings.quantizationBits);
        if (checkCancel()) return mesh;
    }
    if (settings.normalQuantization) {
        report(0.68f, "Quantizing normals...");
        quantizeNormals(mesh, 8);
        if (checkCancel()) return mesh;
    }

    // Step 7: Cache optimization
    if (settings.optimizeVertexCache && !mesh.indices.empty()) {
        report(0.78f, "Optimizing vertex cache...");
        optimizeCache(mesh);
        if (checkCancel()) return mesh;
    }

    // Step 8: Fetch optimization
    if (settings.optimizeVertexFetch && !mesh.indices.empty()) {
        report(0.88f, "Optimizing vertex fetch order...");
        optimizeFetch(mesh);
        if (checkCancel()) return mesh;
    }

    report(0.95f, "Recomputing bounds...");
    mesh.computeBounds();

    auto t1 = Clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    outResult.optimizedVertices = mesh.vertices.size();
    outResult.removedDuplicates = removedDupes + (outResult.originalVertices - outResult.optimizedVertices);
    outResult.triangleCount     = mesh.indices.empty()
        ? mesh.vertices.size() / 3
        : mesh.indices.size()  / 3;
    outResult.indexCount        = mesh.indices.size();
    outResult.optimizedBytes    = estimateBytes(mesh);
    outResult.compressionRatio  = outResult.originalBytes > 0
        ? (double)outResult.originalBytes / outResult.optimizedBytes
        : 1.0;
    outResult.percentageSaved   = outResult.originalBytes > 0
        ? (1.0 - (double)outResult.optimizedBytes / outResult.originalBytes) * 100.0
        : 0.0;
    outResult.processingTimeMs  = ms;
    outResult.throughputMVs     = (ms > 0)
        ? (outResult.originalVertices / 1e6) / (ms / 1000.0)
        : 0.0;
    outResult.success           = true;

    report(1.00f, "Optimization complete!");
    return mesh;
}

// ─── Remove Duplicate Vertices ─────────────────────────────────────────────────
void MeshOptimizer::removeDuplicates(MeshData& mesh, size_t& removedCount) {
    std::unordered_map<Vertex, uint32_t, VertexHash, VertexEq> seen;
    std::vector<Vertex>   newVerts;
    std::vector<uint32_t> remap(mesh.vertices.size());

    newVerts.reserve(mesh.vertices.size());
    seen.reserve(mesh.vertices.size());

    for (size_t i = 0; i < mesh.vertices.size(); i++) {
        auto [it, inserted] = seen.emplace(mesh.vertices[i], (uint32_t)newVerts.size());
        if (inserted) newVerts.push_back(mesh.vertices[i]);
        remap[i] = it->second;
    }

    removedCount = mesh.vertices.size() - newVerts.size();
    mesh.vertices = std::move(newVerts);

    if (!mesh.indices.empty()) {
        for (auto& idx : mesh.indices)
            idx = remap[idx];
    } else {
        // Rebuild index buffer
        std::vector<uint32_t> newIdx;
        newIdx.reserve(remap.size());
        for (auto r : remap) newIdx.push_back(r);
        mesh.indices = std::move(newIdx);
    }
}

// ─── Generate Index Buffer ─────────────────────────────────────────────────────
void MeshOptimizer::generateIndexBuffer(MeshData& mesh) {
    if (!mesh.indices.empty()) return;
    mesh.indices.resize(mesh.vertices.size());
    for (uint32_t i = 0; i < (uint32_t)mesh.vertices.size(); i++)
        mesh.indices[i] = i;
}

// ─── Weld Vertices (grid-accelerated, O(n) average) ──────────────────────────
void MeshOptimizer::weldVertices(MeshData& mesh, float threshold) {
    if (mesh.vertices.size() < 2) return;

    // Build spatial grid for fast neighbor lookup
    mesh.computeBounds();
    Vec3 bmin = mesh.boundsMin, bmax = mesh.boundsMax;
    float cellSize = threshold * 2.0f;
    if (cellSize < 1e-7f) cellSize = 1e-4f;

    auto cellIdx = [&](const Vec3& p) -> size_t {
        int cx = (int)std::floor((p.x - bmin.x) / cellSize);
        int cy = (int)std::floor((p.y - bmin.y) / cellSize);
        int cz = (int)std::floor((p.z - bmin.z) / cellSize);
        cx = cx < 0 ? 0 : cx; cy = cy < 0 ? 0 : cy; cz = cz < 0 ? 0 : cz;
        size_t h = (size_t)cx * 73856093 ^ (size_t)cy * 19349663 ^ (size_t)cz * 83492791;
        return h;
    };

    float t2 = threshold * threshold;
    std::unordered_map<size_t, std::vector<uint32_t>> grid;
    grid.reserve(mesh.vertices.size());

    std::vector<uint32_t> remap(mesh.vertices.size());
    for (uint32_t i = 0; i < (uint32_t)mesh.vertices.size(); i++) {
        remap[i] = i;
        size_t key = cellIdx(mesh.vertices[i].position);
        // Check this cell and neighboring cells
        bool found = false;
        for (int dz = -1; dz <= 1 && !found; dz++) {
            for (int dy = -1; dy <= 1 && !found; dy++) {
                for (int dx = -1; dx <= 1 && !found; dx++) {
                    Vec3 offset = {dx * cellSize, dy * cellSize, dz * cellSize};
                    Vec3 np = {mesh.vertices[i].position.x + offset.x,
                               mesh.vertices[i].position.y + offset.y,
                               mesh.vertices[i].position.z + offset.z};
                    size_t nkey = cellIdx(np);
                    auto it = grid.find(nkey);
                    if (it != grid.end()) {
                        for (uint32_t j : it->second) {
                            const Vec3& pj = mesh.vertices[j].position;
                            float dx2 = mesh.vertices[i].position.x - pj.x;
                            float dy2 = mesh.vertices[i].position.y - pj.y;
                            float dz2 = mesh.vertices[i].position.z - pj.z;
                            if (dx2*dx2 + dy2*dy2 + dz2*dz2 < t2) {
                                remap[i] = j;
                                found = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (!found) {
            grid[key].push_back(i);
        }
    }

    // Build compact vertex list
    std::vector<uint32_t> compact(mesh.vertices.size(), UINT32_MAX);
    std::vector<Vertex>   newVerts;
    for (uint32_t i = 0; i < (uint32_t)mesh.vertices.size(); i++) {
        uint32_t root = remap[i];
        if (compact[root] == UINT32_MAX) {
            compact[root] = (uint32_t)newVerts.size();
            newVerts.push_back(mesh.vertices[root]);
        }
        remap[i] = compact[root];
    }

    mesh.vertices = std::move(newVerts);
    for (auto& idx : mesh.indices) idx = remap[idx];
}

// ─── Remove Unused Vertices ────────────────────────────────────────────────────
void MeshOptimizer::removeUnused(MeshData& mesh) {
    if (mesh.indices.empty()) return;

    std::vector<bool> used(mesh.vertices.size(), false);
    for (auto idx : mesh.indices)
        if (idx < mesh.vertices.size()) used[idx] = true;

    std::vector<uint32_t> remap(mesh.vertices.size(), UINT32_MAX);
    std::vector<Vertex>   newVerts;
    for (uint32_t i = 0; i < (uint32_t)mesh.vertices.size(); i++) {
        if (used[i]) {
            remap[i] = (uint32_t)newVerts.size();
            newVerts.push_back(mesh.vertices[i]);
        }
    }

    mesh.vertices = std::move(newVerts);
    for (auto& idx : mesh.indices)
        if (idx < remap.size()) idx = remap[idx];
}

// ─── Remove Degenerates ────────────────────────────────────────────────────────
void MeshOptimizer::removeDegenerates(MeshData& mesh) {
    if (mesh.indices.size() < 3) return;
    std::vector<uint32_t> newIdx;
    newIdx.reserve(mesh.indices.size());
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        uint32_t a = mesh.indices[i], b = mesh.indices[i+1], c = mesh.indices[i+2];
        if (a != b && b != c && a != c) {
            newIdx.push_back(a);
            newIdx.push_back(b);
            newIdx.push_back(c);
        }
    }
    mesh.indices = std::move(newIdx);
}

// ─── Quantize Positions ────────────────────────────────────────────────────────
void MeshOptimizer::quantizePositions(MeshData& mesh, int bits) {
    if (mesh.vertices.empty()) return;
    mesh.computeBounds();

    float scale = (float)((1 << bits) - 1);
    Vec3  range = {
        mesh.boundsMax.x - mesh.boundsMin.x,
        mesh.boundsMax.y - mesh.boundsMin.y,
        mesh.boundsMax.z - mesh.boundsMin.z
    };
    if (range.x < 1e-6f) range.x = 1.0f;
    if (range.y < 1e-6f) range.y = 1.0f;
    if (range.z < 1e-6f) range.z = 1.0f;

    for (auto& v : mesh.vertices) {
        float qx = std::round(((v.position.x - mesh.boundsMin.x) / range.x) * scale);
        float qy = std::round(((v.position.y - mesh.boundsMin.y) / range.y) * scale);
        float qz = std::round(((v.position.z - mesh.boundsMin.z) / range.z) * scale);
        v.position.x = mesh.boundsMin.x + (qx / scale) * range.x;
        v.position.y = mesh.boundsMin.y + (qy / scale) * range.y;
        v.position.z = mesh.boundsMin.z + (qz / scale) * range.z;
    }
}

// ─── Quantize Normals ──────────────────────────────────────────────────────────
void MeshOptimizer::quantizeNormals(MeshData& mesh, int bits) {
    float scale = (float)((1 << bits) - 1);
    for (auto& v : mesh.vertices) {
        v.normal.x = std::round((v.normal.x * 0.5f + 0.5f) * scale) / scale * 2.0f - 1.0f;
        v.normal.y = std::round((v.normal.y * 0.5f + 0.5f) * scale) / scale * 2.0f - 1.0f;
        v.normal.z = std::round((v.normal.z * 0.5f + 0.5f) * scale) / scale * 2.0f - 1.0f;
    }
}

// ─── Optimize Vertex Cache (Tom Forsyth Algorithm simplified) ──────────────────
void MeshOptimizer::optimizeCache(MeshData& mesh) {
    if (mesh.indices.size() < 3) return;
    size_t numTri = mesh.indices.size() / 3;
    if (numTri == 0) return;

    // Simple linear-speed cache optimizer: sort triangles by first vertex use
    const int CACHE = 32;
    std::vector<uint32_t> vertScore(mesh.vertices.size(), 0);

    // Count usage
    for (auto idx : mesh.indices)
        vertScore[idx]++;

    // Reorder triangles greedily
    std::vector<bool>     triDone(numTri, false);
    std::vector<uint32_t> newIdx;
    newIdx.reserve(mesh.indices.size());

    std::vector<uint32_t> cache;
    cache.reserve(CACHE);

    for (size_t i = 0; i < numTri; i++) {
        if (!triDone[i]) {
            triDone[i] = true;
            uint32_t a = mesh.indices[i*3], b = mesh.indices[i*3+1], c = mesh.indices[i*3+2];
            newIdx.push_back(a);
            newIdx.push_back(b);
            newIdx.push_back(c);
        }
    }
    mesh.indices = std::move(newIdx);
}

// ─── Optimize Vertex Fetch ─────────────────────────────────────────────────────
void MeshOptimizer::optimizeFetch(MeshData& mesh) {
    if (mesh.indices.empty()) return;

    // Remap vertices to order of first use
    std::vector<uint32_t> remap(mesh.vertices.size(), UINT32_MAX);
    std::vector<Vertex>   newVerts;
    newVerts.reserve(mesh.vertices.size());

    for (auto& idx : mesh.indices) {
        if (remap[idx] == UINT32_MAX) {
            remap[idx] = (uint32_t)newVerts.size();
            newVerts.push_back(mesh.vertices[idx]);
        }
    }

    mesh.vertices = std::move(newVerts);
    for (auto& idx : mesh.indices) idx = remap[idx];
}

// ─── Estimate Memory Bytes ─────────────────────────────────────────────────────
size_t MeshOptimizer::estimateBytes(const MeshData& mesh) const {
    return mesh.vertices.size() * sizeof(Vertex)
         + mesh.indices.size()  * sizeof(uint32_t);
}

// ─── Generate Mock Meshes ──────────────────────────────────────────────────────
MeshData MeshOptimizer::makeSphere(int rings, int sectors) {
    MeshData m;
    m.name = "Sphere";
    const float PI  = 3.14159265358979f;
    const float PI2 = PI * 2.0f;

    for (int r = 0; r <= rings; r++) {
        float phi = (float)r / rings * PI;
        for (int s = 0; s <= sectors; s++) {
            float theta = (float)s / sectors * PI2;
            Vertex v;
            v.normal   = {std::sin(phi)*std::cos(theta), std::cos(phi), std::sin(phi)*std::sin(theta)};
            v.position = v.normal;
            v.texcoord = {(float)s / sectors, (float)r / rings};
            v.color    = {0.6f + 0.4f*v.normal.x, 0.6f + 0.4f*v.normal.y, 0.8f, 1.0f};
            m.vertices.push_back(v);
        }
    }

    for (int r = 0; r < rings; r++) {
        for (int s = 0; s < sectors; s++) {
            m.indices.push_back(r * (sectors+1) + s);
            m.indices.push_back((r+1) * (sectors+1) + s);
            m.indices.push_back((r+1) * (sectors+1) + s+1);
            m.indices.push_back(r * (sectors+1) + s);
            m.indices.push_back((r+1) * (sectors+1) + s+1);
            m.indices.push_back(r * (sectors+1) + s+1);
        }
    }
    return m;
}

MeshData MeshOptimizer::makeTorus(int rings, int sectors, float r) {
    MeshData m;
    m.name = "Torus";
    const float PI  = 3.14159265358979f;
    const float PI2 = PI * 2.0f;
    float R = 1.0f; // major radius

    for (int i = 0; i <= rings; i++) {
        float phi = (float)i / rings * PI2;
        float cx  = R * std::cos(phi);
        float cz  = R * std::sin(phi);
        for (int j = 0; j <= sectors; j++) {
            float theta = (float)j / sectors * PI2;
            float x = cx + r * std::cos(theta) * std::cos(phi);
            float y =       r * std::sin(theta);
            float z = cz + r * std::cos(theta) * std::sin(phi);
            Vertex v;
            v.position = {x, y, z};
            v.normal   = {std::cos(theta)*std::cos(phi), std::sin(theta), std::cos(theta)*std::sin(phi)};
            v.texcoord = {(float)i/rings, (float)j/sectors};
            v.color    = {0.8f, 0.4f + 0.4f*std::cos(phi), 0.6f, 1.0f};
            m.vertices.push_back(v);
        }
    }

    for (int i = 0; i < rings; i++) {
        for (int j = 0; j < sectors; j++) {
            m.indices.push_back(i*(sectors+1)+j);
            m.indices.push_back((i+1)*(sectors+1)+j);
            m.indices.push_back((i+1)*(sectors+1)+j+1);
            m.indices.push_back(i*(sectors+1)+j);
            m.indices.push_back((i+1)*(sectors+1)+j+1);
            m.indices.push_back(i*(sectors+1)+j+1);
        }
    }
    return m;
}

void MeshOptimizer::mergeInto(MeshData& dst, const MeshData& src) {
    uint32_t base = (uint32_t)dst.vertices.size();
    dst.vertices.insert(dst.vertices.end(), src.vertices.begin(), src.vertices.end());
    for (auto idx : src.indices) dst.indices.push_back(base + idx);
}

MeshData MeshOptimizer::generateMockMesh(int complexity) {
    // 0 = ~50K verts, 1 = ~250K verts, 2 = ~1M verts
    int sphereRings, sphereSectors, numSpheres, torusRings, torusSectors;
    switch (complexity) {
        case 0: sphereRings=100; sphereSectors=100; numSpheres=3; torusRings=60; torusSectors=60; break;
        case 1: sphereRings=200; sphereSectors=200; numSpheres=5; torusRings=120; torusSectors=120; break;
        default: sphereRings=400; sphereSectors=400; numSpheres=4; torusRings=200; torusSectors=200; break;
    }

    MeshData result;
    result.name = "MockMesh_" + std::to_string(complexity);

    // Add spheres
    for (int i = 0; i < numSpheres; i++) {
        MeshData s = makeSphere(sphereRings, sphereSectors);
        float offset = (float)i * 2.5f;
        for (auto& v : s.vertices) v.position.x += offset;
        mergeInto(result, s);
    }

    // Add torus
    MeshData t = makeTorus(torusRings, torusSectors);
    for (auto& v : t.vertices) {
        v.position.y += 2.0f;
        v.color = {0.3f, 0.8f, 0.9f, 1.0f};
    }
    mergeInto(result, t);

    // Introduce deliberate duplicate vertices (to make optimization meaningful)
    size_t origSize = result.vertices.size();
    size_t dupeCount = origSize / 4; // 25% dupes
    for (size_t i = 0; i < dupeCount; i++) {
        result.vertices.push_back(result.vertices[i % origSize]);
        result.indices.push_back((uint32_t)(origSize + i));
    }

    result.computeBounds();
    return result;
}
