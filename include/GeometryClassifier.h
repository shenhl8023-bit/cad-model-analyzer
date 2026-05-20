#pragma once

#include <TopoDS_Shape.hxx>

struct CurveStats {
    int line = 0;
    int circle = 0;
    int ellipse = 0;
    int bsplineCurve = 0;
    int other = 0;
};

struct SurfaceStats {
    int plane = 0;
    int cylinder = 0;
    int sphere = 0;
    int cone = 0;
    int torus = 0;
    int bsplineSurface = 0;
    int other = 0;
};

CurveStats classifyCurves(const TopoDS_Shape& shape);
SurfaceStats classifySurfaces(const TopoDS_Shape& shape);
