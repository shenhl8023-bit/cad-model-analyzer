#include "QualityAssessment.h"

namespace {
std::string qualityStatus(const TopologyStats& topology, const ShapeMetrics& metrics) {
    if (topology.vertex == 0 && topology.edge == 0 && topology.face == 0 && topology.solid == 0) {
        return "empty";
    }
    if (topology.nonManifoldEdge > 0) {
        return "non_manifold";
    }
    if (topology.freeEdge > 0) {
        return "open_shell";
    }
    if (topology.solid > 1) {
        return "multi_solid";
    }
    if (topology.solid == 0 && topology.shell > 0) {
        return "shell_only";
    }
    if (topology.solid > 0 && metrics.volume <= 0.0) {
        return "zero_or_invalid_volume";
    }
    return "ok";
}

std::string complexityLevel(const TopologyStats& topology) {
    const int complexityScore = topology.face + topology.edge + topology.vertex;
    if (complexityScore >= 2000 || topology.face >= 500) {
        return "high";
    }
    if (complexityScore >= 300 || topology.face >= 100) {
        return "medium";
    }
    return "low";
}
}

QualityAssessment assessQuality(const TopologyStats& topology, const ShapeMetrics& metrics) {
    QualityAssessment quality;
    quality.hasFreeEdges = topology.freeEdge > 0;
    quality.hasNonManifoldEdges = topology.nonManifoldEdge > 0;
    quality.hasPositiveVolume = metrics.volume > 0.0;
    quality.isEmpty = topology.vertex == 0 && topology.edge == 0 && topology.face == 0 && topology.solid == 0;
    quality.multiSolid = topology.solid > 1;
    quality.shellOnly = topology.solid == 0 && topology.shell > 0;
    quality.closedSolidCandidate =
        topology.solid == 1 &&
        !quality.hasFreeEdges &&
        !quality.hasNonManifoldEdges &&
        quality.hasPositiveVolume;
    quality.issueCount =
        (quality.isEmpty ? 1 : 0) +
        (quality.multiSolid ? 1 : 0) +
        (quality.shellOnly ? 1 : 0) +
        (quality.hasFreeEdges ? 1 : 0) +
        (quality.hasNonManifoldEdges ? 1 : 0) +
        (topology.solid > 0 && !quality.hasPositiveVolume ? 1 : 0);
    quality.status = qualityStatus(topology, metrics);
    quality.complexityLevel = complexityLevel(topology);
    return quality;
}
