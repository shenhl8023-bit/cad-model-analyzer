#include "TopologyCounter.h"

#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

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
    stats.eulerCharacteristic = stats.vertex - stats.edge + stats.face;

    TopTools_IndexedDataMapOfShapeListOfShape edgeFaceMap;
    TopExp::MapShapesAndAncestors(shape, TopAbs_EDGE, TopAbs_FACE, edgeFaceMap);
    for (int i = 1; i <= edgeFaceMap.Extent(); ++i) {
        const int adjacentFaceCount = edgeFaceMap.FindFromIndex(i).Extent();
        if (adjacentFaceCount < 2) {
            ++stats.freeEdge;
        } else if (adjacentFaceCount > 2) {
            ++stats.nonManifoldEdge;
        }
    }

    return stats;
}
