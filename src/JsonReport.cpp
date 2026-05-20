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
    const TopologyStats& topology,
    const CurveStats& curves,
    const SurfaceStats& surfaces,
    const ShapeMetrics& metrics) {
    std::ostringstream json;
    json << "{\n";
    json << "  \"file\": \"" << jsonEscape(inputFile) << "\",\n";
    json << "  \"topology\": {\n";
    json << "    \"vertex\": " << topology.vertex << ",\n";
    json << "    \"edge\": " << topology.edge << ",\n";
    json << "    \"wire\": " << topology.wire << ",\n";
    json << "    \"face\": " << topology.face << ",\n";
    json << "    \"shell\": " << topology.shell << ",\n";
    json << "    \"solid\": " << topology.solid << ",\n";
    json << "    \"compound\": " << topology.compound << "\n";
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
