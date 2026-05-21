#include "QualityAssessment.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

ShapeMetrics metricsWithVolume(double volume) {
    ShapeMetrics metrics;
    metrics.volume = volume;
    return metrics;
}

void closedSolidHasOkQuality() {
    TopologyStats topology;
    topology.vertex = 26;
    topology.edge = 44;
    topology.face = 10;
    topology.solid = 1;

    const QualityAssessment quality = assessQuality(topology, metricsWithVolume(123.0));

    expect(quality.closedSolidCandidate, "closed solid should be a closed_solid_candidate");
    expect(!quality.hasFreeEdges, "closed solid should not have free edges");
    expect(!quality.hasNonManifoldEdges, "closed solid should not have non-manifold edges");
    expect(quality.hasPositiveVolume, "closed solid should have positive volume");
    expect(!quality.isEmpty, "closed solid should not be empty");
    expect(!quality.multiSolid, "single solid should not be multi_solid");
    expect(!quality.shellOnly, "solid should not be shell_only");
    expect(quality.issueCount == 0, "closed solid should have no issues");
    expect(quality.status == "ok", "closed solid status should be ok");
    expect(quality.complexityLevel == "low", "small screw fixture should be low complexity");
}

void freeEdgesAreReportedAsOpenShellIssue() {
    TopologyStats topology;
    topology.vertex = 10;
    topology.edge = 12;
    topology.face = 2;
    topology.shell = 1;
    topology.freeEdge = 4;

    const QualityAssessment quality = assessQuality(topology, metricsWithVolume(0.0));

    expect(!quality.closedSolidCandidate, "open shell should not be a closed_solid_candidate");
    expect(quality.hasFreeEdges, "free edge flag should be true");
    expect(!quality.hasNonManifoldEdges, "non-manifold flag should be false");
    expect(!quality.hasPositiveVolume, "zero volume should not be positive");
    expect(!quality.isEmpty, "open shell should not be empty");
    expect(!quality.multiSolid, "open shell should not be multi_solid");
    expect(quality.shellOnly, "shape with shell and no solid should be shell_only");
    expect(quality.issueCount == 2, "open shell should count free edges and shell_only issues");
    expect(quality.status == "open_shell", "free edges should take open_shell status");
}

void nonManifoldTakesStatusPrecedence() {
    TopologyStats topology;
    topology.vertex = 20;
    topology.edge = 30;
    topology.face = 12;
    topology.solid = 2;
    topology.freeEdge = 3;
    topology.nonManifoldEdge = 1;

    const QualityAssessment quality = assessQuality(topology, metricsWithVolume(50.0));

    expect(quality.hasFreeEdges, "free edge flag should remain true");
    expect(quality.hasNonManifoldEdges, "non-manifold flag should be true");
    expect(quality.multiSolid, "two solids should set multi_solid");
    expect(quality.issueCount == 3, "should count multi_solid, free edge, and non-manifold issues");
    expect(quality.status == "non_manifold", "non-manifold status should take precedence over free edges");
}

void complexityThresholdsAreStable() {
    TopologyStats medium;
    medium.vertex = 100;
    medium.edge = 150;
    medium.face = 50;
    expect(assessQuality(medium, metricsWithVolume(0.0)).complexityLevel == "medium", "score 300 should be medium complexity");

    TopologyStats high;
    high.vertex = 700;
    high.edge = 800;
    high.face = 500;
    expect(assessQuality(high, metricsWithVolume(0.0)).complexityLevel == "high", "500 faces should be high complexity");
}
}

int main() {
    closedSolidHasOkQuality();
    freeEdgesAreReportedAsOpenShellIssue();
    nonManifoldTakesStatusPrecedence();
    complexityThresholdsAreStable();
    std::cout << "quality_assessment_tests OK\n";
    return 0;
}
