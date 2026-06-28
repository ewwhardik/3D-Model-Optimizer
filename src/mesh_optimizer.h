#pragma once
#include "types.h"
#include <functional>
#include <atomic>

class MeshOptimizer {
public:
    using ProgressCallback = std::function<void(float, const std::string&)>;

    // Run full optimization pipeline
    MeshData optimize(const MeshData& input,
                      const OptimizeSettings& settings,
                      OptimizeResult& outResult,
                      ProgressCallback progress = nullptr,
                      std::atomic<bool>* cancel = nullptr);

    // Generate procedural test mesh
    static MeshData generateMockMesh(int complexity); // 0=50K, 1=250K, 2=1M

private:
    void  removeDuplicates(MeshData& mesh, size_t& removedCount);
    void  generateIndexBuffer(MeshData& mesh);
    void  weldVertices(MeshData& mesh, float threshold = 1e-5f);
    void  removeUnused(MeshData& mesh);
    void  removeDegenerates(MeshData& mesh);
    void  quantizePositions(MeshData& mesh, int bits);
    void  quantizeNormals(MeshData& mesh, int bits);
    void  optimizeCache(MeshData& mesh);
    void  optimizeFetch(MeshData& mesh);

    size_t estimateBytes(const MeshData& mesh) const;
    static MeshData makeSphere(int rings, int sectors);
    static MeshData makeTorus(int rings, int sectors, float r = 0.3f);
    static void mergeInto(MeshData& dst, const MeshData& src);
};
