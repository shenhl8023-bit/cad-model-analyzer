#include "ShapeMetrics.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>

#include <cmath>

ShapeMetrics measureShape(const TopoDS_Shape& shape) {
    ShapeMetrics metrics;

    Bnd_Box box;
    BRepBndLib::Add(shape, box);
    box.Get(
        metrics.boundingBox.xmin,
        metrics.boundingBox.ymin,
        metrics.boundingBox.zmin,
        metrics.boundingBox.xmax,
        metrics.boundingBox.ymax,
        metrics.boundingBox.zmax);

    metrics.boundingBox.dx = metrics.boundingBox.xmax - metrics.boundingBox.xmin;
    metrics.boundingBox.dy = metrics.boundingBox.ymax - metrics.boundingBox.ymin;
    metrics.boundingBox.dz = metrics.boundingBox.zmax - metrics.boundingBox.zmin;
    metrics.boundingBox.centerX = (metrics.boundingBox.xmin + metrics.boundingBox.xmax) / 2.0;
    metrics.boundingBox.centerY = (metrics.boundingBox.ymin + metrics.boundingBox.ymax) / 2.0;
    metrics.boundingBox.centerZ = (metrics.boundingBox.zmin + metrics.boundingBox.zmax) / 2.0;
    metrics.boundingBox.diagonal = std::sqrt(
        metrics.boundingBox.dx * metrics.boundingBox.dx +
        metrics.boundingBox.dy * metrics.boundingBox.dy +
        metrics.boundingBox.dz * metrics.boundingBox.dz);

    GProp_GProps surfaceProps;
    BRepGProp::SurfaceProperties(shape, surfaceProps);
    metrics.surfaceArea = surfaceProps.Mass();

    GProp_GProps volumeProps;
    BRepGProp::VolumeProperties(shape, volumeProps);
    metrics.volume = volumeProps.Mass();
    const gp_Pnt centerOfMass = volumeProps.CentreOfMass();
    metrics.centerOfMass.x = centerOfMass.X();
    metrics.centerOfMass.y = centerOfMass.Y();
    metrics.centerOfMass.z = centerOfMass.Z();

    return metrics;
}
