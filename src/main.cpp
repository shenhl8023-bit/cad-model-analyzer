#include "Analysis.h"
#include "JsonReport.h"
#include "QualityAssessment.h"

#include <windows.h>

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
constexpr const char* kVersion = "0.5.0";

struct CliOptions {
    std::string inputFile;
    std::string outputFile = "report.json";
    bool batchMode = false;
    bool showHelp = false;
    bool showVersion = false;
};

void printUsage(const char* exeName) {
    std::cout << "CAD Model Analyzer " << kVersion << "\n\n"
              << "Analyze STEP/STP CAD models and export structured JSON/CSV reports.\n\n"
              << "Usage:\n"
              << "  " << exeName << " <input.step|input.stp> [output.json]\n"
              << "  " << exeName << " <input.step|input.stp> -o <output.json>\n"
              << "  " << exeName << " --batch <input_dir> -o <output_dir>\n\n"
              << "Options:\n"
              << "  -o <file|dir>  Single-file JSON output path, or batch output directory.\n"
              << "                 Default: report.json for single-file mode; batch-report for batch mode.\n"
              << "  --batch <dir>  Analyze all .step/.stp files directly under a directory.\n"
              << "                 Outputs one JSON report per model plus summary.csv.\n"
              << "  --help, -h     Show this help message.\n"
              << "  --version      Show analyzer version.\n\n"
              << "Report contents:\n"
              << "  metadata       Analyzer version, input path, and analysis time.\n"
              << "  topology       Vertex/edge/face/solid counts, edge-face adjacency, Euler characteristic.\n"
              << "  curves         Line/circle/ellipse/B-spline curve counts.\n"
              << "  surfaces       Plane/cylinder/sphere/cone/torus/B-spline surface counts.\n"
              << "  metrics        Bounding box, surface area, volume, and center of mass.\n"
              << "  quality        Closed-solid candidate, issue count, status, and complexity level.\n\n"
              << "Quality status values:\n"
              << "  ok             No basic topology/volume issue detected.\n"
              << "  empty          No solid, shell, face, edge, or vertex found.\n"
              << "  open_shell     Free/boundary edges are present.\n"
              << "  non_manifold   At least one edge is shared by more than two faces.\n"
              << "  multi_solid    More than one solid exists in the model.\n"
              << "  shell_only     Shell/face geometry exists but no solid exists.\n"
              << "  invalid_volume Solid exists but calculated volume is not positive.\n\n"
              << "Exit codes:\n"
              << "  0  Success.\n"
              << "  1  Invalid CLI arguments or single-file analysis failure.\n"
              << "  2  Batch mode completed with one or more file-level failures.\n\n"
              << "Examples:\n"
              << "  " << exeName << " part.step -o report.json\n"
              << "  " << exeName << " part.step report.json\n"
              << "  " << exeName << " --batch samples -o output/batch-report\n";
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
        if (arg == "--batch") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Missing directory after --batch");
            }
            options.batchMode = true;
            options.inputFile = argv[++i];
            if (options.outputFile == "report.json") {
                options.outputFile = "batch-report";
            }
            continue;
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
        if (!options.batchMode && options.outputFile == "report.json") {
            options.outputFile = arg;
            continue;
        }
        throw std::runtime_error("Unexpected argument: " + arg);
    }

    if (!options.showHelp && !options.showVersion && options.inputFile.empty()) {
        throw std::runtime_error("Missing input STEP file or --batch directory");
    }

    return options;
}

bool hasStepExtension(const std::filesystem::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return ext == ".step" || ext == ".stp";
}

std::string csvEscape(const std::string& value) {
    const bool needsQuotes = value.find_first_of(",\"\r\n") != std::string::npos;
    if (!needsQuotes) {
        return value;
    }
    std::ostringstream escaped;
    escaped << '"';
    for (const char c : value) {
        if (c == '"') {
            escaped << "\"\"";
        } else {
            escaped << c;
        }
    }
    escaped << '"';
    return escaped.str();
}

std::string outputJsonPathFor(const std::filesystem::path& outputDir, const std::filesystem::path& inputFile) {
    std::filesystem::path outputPath = outputDir / inputFile.stem();
    outputPath.replace_extension(".json");
    return outputPath.string();
}

int runSingleFile(const CliOptions& options) {
    const AnalysisResult result = analyzeStepFile(options.inputFile);
    const std::string report = buildJsonReport(
        result.inputFile,
        kVersion,
        result.analysisTimeMs,
        result.topology,
        result.curves,
        result.surfaces,
        result.metrics);

    writeTextFile(options.outputFile, report);
    std::cout << report;
    std::cerr << "Report written to: " << options.outputFile << "\n";
    return 0;
}

int runBatch(const CliOptions& options) {
    const std::filesystem::path inputDir(options.inputFile);
    const std::filesystem::path outputDir(options.outputFile);

    if (!std::filesystem::is_directory(inputDir)) {
        throw std::runtime_error("Batch input is not a directory: " + inputDir.string());
    }

    std::filesystem::create_directories(outputDir);

    std::vector<std::filesystem::path> inputFiles;
    for (const auto& entry : std::filesystem::directory_iterator(inputDir)) {
        if (entry.is_regular_file() && hasStepExtension(entry.path())) {
            inputFiles.push_back(entry.path());
        }
    }
    std::sort(inputFiles.begin(), inputFiles.end());

    std::ostringstream summary;
    summary << "file,status,quality_status,complexity_level,issue_count,solid,face,edge,free_edge,manifold_edge,non_manifold_edge,max_edge_face_adjacency,volume,surface_area,analysis_time_ms,report\n";

    int failures = 0;
    for (const auto& inputFile : inputFiles) {
        const std::string inputPath = inputFile.string();
        const std::string reportPath = outputJsonPathFor(outputDir, inputFile);
        try {
            const AnalysisResult result = analyzeStepFile(inputPath);
            const std::string report = buildJsonReport(
                result.inputFile,
                kVersion,
                result.analysisTimeMs,
                result.topology,
                result.curves,
                result.surfaces,
                result.metrics);
            writeTextFile(reportPath, report);

            const QualityAssessment quality = assessQuality(result.topology, result.metrics);

            summary << csvEscape(inputFile.filename().string()) << ",ok,"
                    << quality.status << ','
                    << quality.complexityLevel << ','
                    << quality.issueCount << ','
                    << result.topology.solid << ','
                    << result.topology.face << ','
                    << result.topology.edge << ','
                    << result.topology.freeEdge << ','
                    << result.topology.manifoldEdge << ','
                    << result.topology.nonManifoldEdge << ','
                    << result.topology.maxEdgeFaceAdjacency << ','
                    << result.metrics.volume << ','
                    << result.metrics.surfaceArea << ','
                    << result.analysisTimeMs << ','
                    << csvEscape(std::filesystem::path(reportPath).filename().string()) << "\n";
        } catch (const std::exception& ex) {
            ++failures;
            summary << csvEscape(inputFile.filename().string()) << ",error,,,,,,,,,,,,,,"
                    << csvEscape(ex.what()) << "\n";
            std::cerr << "Error analyzing " << inputPath << ": " << ex.what() << "\n";
        }
    }

    const std::filesystem::path summaryPath = outputDir / "summary.csv";
    writeTextFile(summaryPath.string(), summary.str());
    std::cout << "Batch analyzed " << inputFiles.size() << " STEP files, failures: " << failures << "\n";
    std::cout << "Summary written to: " << summaryPath.string() << "\n";
    return failures == 0 ? 0 : 2;
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
        if (options.batchMode) {
            return runBatch(options);
        }
        return runSingleFile(options);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n\n";
        printUsage(argv[0]);
        return 1;
    }
}
