#include "GeometryClassifier.h"

#include <BRep_Tool.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Circle.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Standard_Type.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

CurveStats classifyCurves(const TopoDS_Shape& shape) {
    CurveStats stats;

    for (TopExp_Explorer explorer(shape, TopAbs_EDGE); explorer.More(); explorer.Next()) {
        const TopoDS_Edge edge = TopoDS::Edge(explorer.Current());
        Standard_Real first = 0.0;
        Standard_Real last = 0.0;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
        if (curve.IsNull()) {
            ++stats.other;
        } else if (curve->IsKind(STANDARD_TYPE(Geom_Line))) {
            ++stats.line;
        } else if (curve->IsKind(STANDARD_TYPE(Geom_Circle))) {
            ++stats.circle;
        } else if (curve->IsKind(STANDARD_TYPE(Geom_Ellipse))) {
            ++stats.ellipse;
        } else if (curve->IsKind(STANDARD_TYPE(Geom_BSplineCurve))) {
            ++stats.bsplineCurve;
        } else {
            ++stats.other;
        }
    }

    return stats;
}

SurfaceStats classifySurfaces(const TopoDS_Shape& shape) {
    SurfaceStats stats;

    for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
        const TopoDS_Face face = TopoDS::Face(explorer.Current());
        Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
        if (surface.IsNull()) {
            ++stats.other;
        } else if (surface->IsKind(STANDARD_TYPE(Geom_Plane))) {
            ++stats.plane;
        } else if (surface->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))) {
            ++stats.cylinder;
        } else if (surface->IsKind(STANDARD_TYPE(Geom_SphericalSurface))) {
            ++stats.sphere;
        } else if (surface->IsKind(STANDARD_TYPE(Geom_ConicalSurface))) {
            ++stats.cone;
        } else if (surface->IsKind(STANDARD_TYPE(Geom_ToroidalSurface))) {
            ++stats.torus;
        } else if (surface->IsKind(STANDARD_TYPE(Geom_BSplineSurface))) {
            ++stats.bsplineSurface;
        } else {
            ++stats.other;
        }
    }

    return stats;
}
