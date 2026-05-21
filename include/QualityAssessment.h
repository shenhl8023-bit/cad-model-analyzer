#pragma once

#include "ShapeMetrics.h"
#include "TopologyCounter.h"

#include <string>

struct QualityAssessment {
    bool closedSolidCandidate = false;
    bool hasFreeEdges = false;
    bool hasNonManifoldEdges = false;
    bool hasPositiveVolume = false;
    bool isEmpty = false;
    bool multiSolid = false;
    bool shellOnly = false;
    int issueCount = 0;
    std::string status;
    std::string complexityLevel;
};

QualityAssessment assessQuality(const TopologyStats& topology, const ShapeMetrics& metrics);
