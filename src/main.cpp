#include "GeometryClassifier.h"
#include "JsonReport.h"
#include "ShapeMetrics.h"
#include "StepReader.h"
#include "TopologyCounter.h"

#include <TopoDS_Shape.hxx>

#include <windows.h>

#include <chrono>
#include <exception>
#include <iostream>
#include <string>

namespace {
constexpr const char* kVersion = "0.2.0";

struct CliOptions {
    std::string inputFile;
    std::string outputFile = "report.json";
    bool showHelp = false;
    bool showVersion = false;
};

void printUsage(const char* exeName) {
    std::cout << "Usage:\n"
              << "  " << exeName << " <input.step|input.stp> [output.json]\n"
              << "  " << exeName << " <input.step|input.stp> -o <output.json>\n\n"
              << "Options:\n"
              << "  -o <file>     Write report to file\n"
              << "  --help        Show help\n"
              << "  --version     Show version\n\n"
              << "Example:\n"
              << "  " << exeName << " part.step -o report.json\n";
}

CliOptions parseArgs(int argc, char** argv) {
    CliOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            options.showHelp = true;
            return options;
        }
        if (arg == "--version") {
            options.showVersion = true;
            return options;
        }
        if (arg == "-o") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Missing value after -o");
            }
            options.outputFile = argv[++i];
            continue;
        }
        if (options.inputFile.empty()) {
            options.inputFile = arg;
            continue;
        }
        if (options.outputFile == "report.json") {
            options.outputFile = arg;
            continue;
        }
        throw std::runtime_error("Unexpected argument: " + arg);
    }

    if (options.inputFile.empty()) {
        throw std::runtime_error("Missing input STEP file");
    }

    return options;
}
}

int main(int argc, char** argv) {
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);

    try {
        const CliOptions options = parseArgs(argc, argv);
        if (options.showHelp) {
            printUsage(argv[0]);
            return 0;
        }
        if (options.showVersion) {
            std::cout << "cad_model_analyzer " << kVersion << "\n";
            return 0;
        }

        const auto analysisStart = std::chrono::steady_clock::now();

        const TopoDS_Shape shape = readStepFile(options.inputFile);
        const TopologyStats topology = countTopology(shape);
        const CurveStats curves = classifyCurves(shape);
        const SurfaceStats surfaces = classifySurfaces(shape);
        const ShapeMetrics metrics = measureShape(shape);
        const auto analysisEnd = std::chrono::steady_clock::now();
        const auto analysisTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(analysisEnd - analysisStart).count();
        const std::string report = buildJsonReport(options.inputFile, kVersion, analysisTimeMs, topology, curves, surfaces, metrics);

        writeTextFile(options.outputFile, report);
        std::cout << report;
        std::cerr << "Report written to: " << options.outputFile << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n\n";
        printUsage(argv[0]);
        return 1;
    }
}
