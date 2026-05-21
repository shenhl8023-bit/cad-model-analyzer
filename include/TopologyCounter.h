#pragma once

#include <TopoDS_Shape.hxx>

struct TopologyStats {
    int vertex = 0;
    int edge = 0;
    int wire = 0;
    int face = 0;
    int shell = 0;
    int solid = 0;
    int compound = 0;
    int isolatedEdge = 0;
    int freeEdge = 0;
    int manifoldEdge = 0;
    int nonManifoldEdge = 0;
    int maxEdgeFaceAdjacency = 0;
    int eulerCharacteristic = 0;
};

TopologyStats countTopology(const TopoDS_Shape& shape);
