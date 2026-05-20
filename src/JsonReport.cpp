#include "JsonReport.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
std::string jsonEscape(const std::string& value) {
    std::ostringstream out;
    for (const char c : value) {
        switch (c) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break;
        case '\f': out << "\\f"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default: out << c; break;
        }
    }
    return out.str();
}
}

std::string buildJsonReport(
    const std::string& inputFile,
    const std::string& analyzerVersion,
    const long long analysisTimeMs,
    const TopologyStats& topology,
    const CurveStats& curves,
    const SurfaceStats& surfaces,
    const ShapeMetrics& metrics) {
    const bool closedSolidCandidate =
        topology.solid == 1 &&
        topology.freeEdge == 0 &&
        topology.nonManifoldEdge == 0 &&
        metrics.volume > 0.0;

    std::ostringstream json;
    json << "{\n";
    json << "  \"metadata\": {\n";
    json << "    \"analyzer\": \"cad_model_analyzer\",\n";
    json << "    \"version\": \"" << jsonEscape(analyzerVersion) << "\",\n";
    json << "    \"input_file\": \"" << jsonEscape(inputFile) << "\",\n";
    json << "    \"analysis_time_ms\": " << analysisTimeMs << "\n";
    json << "  },\n";
    json << "  \"file\": \"" << jsonEscape(inputFile) << "\",\n";
    json << "  \"topology\": {\n";
    json << "    \"vertex\": " << topology.vertex << ",\n";
    json << "    \"edge\": " << topology.edge << ",\n";
    json << "    \"wire\": " << topology.wire << ",\n";
    json << "    \"face\": " << topology.face << ",\n";
    json << "    \"shell\": " << topology.shell << ",\n";
    json << "    \"solid\": " << topology.solid << ",\n";
    json << "    \"compound\": " << topology.compound << ",\n";
    json << "    \"free_edge\": " << topology.freeEdge << ",\n";
    json << "    \"non_manifold_edge\": " << topology.nonManifoldEdge << ",\n";
    json << "    \"euler_characteristic\": " << topology.eulerCharacteristic << "\n";
    json << "  },\n";
    json << "  \"curves\": {\n";
    json << "    \"line\": " << curves.line << ",\n";
    json << "    \"circle\": " << curves.circle << ",\n";
    json << "    \"ellipse\": " << curves.ellipse << ",\n";
    json << "    \"bspline_curve\": " << curves.bsplineCurve << ",\n";
    json << "    \"other\": " << curves.other << "\n";
    json << "  },\n";
    json << "  \"surfaces\": {\n";
    json << "    \"plane\": " << surfaces.plane << ",\n";
    json << "    \"cylinder\": " << surfaces.cylinder << ",\n";
    json << "    \"sphere\": " << surfaces.sphere << ",\n";
    json << "    \"cone\": " << surfaces.cone << ",\n";
    json << "    \"torus\": " << surfaces.torus << ",\n";
    json << "    \"bspline_surface\": " << surfaces.bsplineSurface << ",\n";
    json << "    \"other\": " << surfaces.other << "\n";
    json << "  },\n";
    json << "  \"metrics\": {\n";
    json << "    \"bounding_box\": {\n";
    json << "      \"xmin\": " << metrics.boundingBox.xmin << ",\n";
    json << "      \"ymin\": " << metrics.boundingBox.ymin << ",\n";
    json << "      \"zmin\": " << metrics.boundingBox.zmin << ",\n";
    json << "      \"xmax\": " << metrics.boundingBox.xmax << ",\n";
    json << "      \"ymax\": " << metrics.boundingBox.ymax << ",\n";
    json << "      \"zmax\": " << metrics.boundingBox.zmax << ",\n";
    json << "      \"dx\": " << metrics.boundingBox.dx << ",\n";
    json << "      \"dy\": " << metrics.boundingBox.dy << ",\n";
    json << "      \"dz\": " << metrics.boundingBox.dz << ",\n";
    json << "      \"center\": {\n";
    json << "        \"x\": " << metrics.boundingBox.centerX << ",\n";
    json << "        \"y\": " << metrics.boundingBox.centerY << ",\n";
    json << "        \"z\": " << metrics.boundingBox.centerZ << "\n";
    json << "      },\n";
    json << "      \"diagonal\": " << metrics.boundingBox.diagonal << "\n";
    json << "    },\n";
    json << "    \"surface_area\": " << metrics.surfaceArea << ",\n";
    json << "    \"volume\": " << metrics.volume << ",\n";
    json << "    \"center_of_mass\": {\n";
    json << "      \"x\": " << metrics.centerOfMass.x << ",\n";
    json << "      \"y\": " << metrics.centerOfMass.y << ",\n";
    json << "      \"z\": " << metrics.centerOfMass.z << "\n";
    json << "    }\n";
    json << "  },\n";
    json << "  \"quality\": {\n";
    json << "    \"closed_solid_candidate\": " << (closedSolidCandidate ? "true" : "false") << ",\n";
    json << "    \"has_free_edges\": " << (topology.freeEdge > 0 ? "true" : "false") << ",\n";
    json << "    \"has_non_manifold_edges\": " << (topology.nonManifoldEdge > 0 ? "true" : "false") << ",\n";
    json << "    \"has_positive_volume\": " << (metrics.volume > 0.0 ? "true" : "false") << "\n";
    json << "  }\n";
    json << "}\n";
    return json.str();
}

void writeTextFile(const std::string& path, const std::string& text) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Failed to open output file: " + path);
    }
    output << text;
}
