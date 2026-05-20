#pragma once

#include <TopoDS_Shape.hxx>

struct BoundingBoxMetrics {
    double xmin = 0.0;
    double ymin = 0.0;
    double zmin = 0.0;
    double xmax = 0.0;
    double ymax = 0.0;
    double zmax = 0.0;
    double dx = 0.0;
    double dy = 0.0;
    double dz = 0.0;
    double centerX = 0.0;
    double centerY = 0.0;
    double centerZ = 0.0;
    double diagonal = 0.0;
};

struct PointMetrics {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct ShapeMetrics {
    BoundingBoxMetrics boundingBox;
    PointMetrics centerOfMass;
    double surfaceArea = 0.0;
    double volume = 0.0;
};

ShapeMetrics measureShape(const TopoDS_Shape& shape);
