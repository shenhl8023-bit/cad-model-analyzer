#include "StepReader.h"

#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <TopoDS_Shape.hxx>

#include <stdexcept>
#include <string>

TopoDS_Shape readStepFile(const std::string& path) {
    STEPControl_Reader reader;
    const IFSelect_ReturnStatus status = reader.ReadFile(path.c_str());
    if (status != IFSelect_RetDone) {
        throw std::runtime_error("Failed to read STEP file: " + path);
    }

    const Standard_Integer transferred = reader.TransferRoots();
    if (transferred <= 0) {
        throw std::runtime_error("No transferable roots in STEP file: " + path);
    }

    TopoDS_Shape shape = reader.OneShape();
    if (shape.IsNull()) {
        throw std::runtime_error("STEP file produced a null shape: " + path);
    }
    return shape;
}
