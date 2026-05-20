#pragma once

#include "GeometryClassifier.h"
#include "ShapeMetrics.h"
#include "TopologyCounter.h"

#include <string>

std::string buildJsonReport(
    const std::string& inputFile,
    const TopologyStats& topology,
    const CurveStats& curves,
    const SurfaceStats& surfaces,
    const ShapeMetrics& metrics);

void writeTextFile(const std::string& path, const std::string& text);
