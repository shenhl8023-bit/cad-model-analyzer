#include "TopologyCounter.h"

#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>

namespace {
int countShapeType(const TopoDS_Shape& shape, TopAbs_ShapeEnum type) {
    int count = 0;
    for (TopExp_Explorer explorer(shape, type); explorer.More(); explorer.Next()) {
        ++count;
    }
    return count;
}
}

TopologyStats countTopology(const TopoDS_Shape& shape) {
    TopologyStats stats;
    stats.vertex = countShapeType(shape, TopAbs_VERTEX);
    stats.edge = countShapeType(shape, TopAbs_EDGE);
    stats.wire = countShapeType(shape, TopAbs_WIRE);
    stats.face = countShapeType(shape, TopAbs_FACE);
    stats.shell = countShapeType(shape, TopAbs_SHELL);
    stats.solid = countShapeType(shape, TopAbs_SOLID);
    stats.compound = countShapeType(shape, TopAbs_COMPOUND);
    return stats;
}
