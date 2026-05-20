#include "Analysis.h"
#include "JsonReport.h"

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
constexpr const char* kVersion = "0.3.0";

struct CliOptions {
    std::string inputFile;
    std::string outputFile = "report.json";
    bool batchMode = false;
    bool showHelp = false;
    bool showVersion = false;
};

void printUsage(const char* exeName) {
    std::cout << "Usage:\n"
              << "  " << exeName << " <input.step|input.stp> [output.json]\n"
              << "  " << exeName << " <input.step|input.stp> -o <output.json>\n"
              << "  " << exeName << " --batch <input_dir> -o <output_dir>\n\n"
              << "Options:\n"
              << "  -o <file|dir>  Write report file, or batch output directory\n"
              << "  --batch <dir>  Analyze all .step/.stp files in a directory\n"
              << "  --help         Show help\n"
              << "  --version      Show version\n\n"
              << "Examples:\n"
              << "  " << exeName << " part.step -o report.json\n"
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
    summary << "file,status,solid,face,edge,free_edge,non_manifold_edge,volume,surface_area,analysis_time_ms,report\n";

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

            summary << csvEscape(inputFile.filename().string()) << ",ok,"
                    << result.topology.solid << ','
                    << result.topology.face << ','
                    << result.topology.edge << ','
                    << result.topology.freeEdge << ','
                    << result.topology.nonManifoldEdge << ','
                    << result.metrics.volume << ','
                    << result.metrics.surfaceArea << ','
                    << result.analysisTimeMs << ','
                    << csvEscape(std::filesystem::path(reportPath).filename().string()) << "\n";
        } catch (const std::exception& ex) {
            ++failures;
            summary << csvEscape(inputFile.filename().string()) << ",error,,,,,,,,,"
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
