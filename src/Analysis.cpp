#include "Analysis.h"

#include "StepReader.h"

#include <TopoDS_Shape.hxx>

#include <chrono>

AnalysisResult analyzeStepFile(const std::string& inputFile) {
    const auto analysisStart = std::chrono::steady_clock::now();

    const TopoDS_Shape shape = readStepFile(inputFile);

    AnalysisResult result;
    result.inputFile = inputFile;
    result.topology = countTopology(shape);
    result.curves = classifyCurves(shape);
    result.surfaces = classifySurfaces(shape);
    result.metrics = measureShape(shape);

    const auto analysisEnd = std::chrono::steady_clock::now();
    result.analysisTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(analysisEnd - analysisStart).count();
    return result;
}
