#pragma once

#include "GeometryClassifier.h"
#include "ShapeMetrics.h"
#include "TopologyCounter.h"

#include <string>

struct AnalysisResult {
    std::string inputFile;
    long long analysisTimeMs = 0;
    TopologyStats topology;
    CurveStats curves;
    SurfaceStats surfaces;
    ShapeMetrics metrics;
};

AnalysisResult analyzeStepFile(const std::string& inputFile);

std::string buildJsonReport(
    const std::string& inputFile,
    const std::string& analyzerVersion,
    long long analysisTimeMs,
    const TopologyStats& topology,
    const CurveStats& curves,
    const SurfaceStats& surfaces,
    const ShapeMetrics& metrics);

void writeTextFile(const std::string& path, const std::string& text);
