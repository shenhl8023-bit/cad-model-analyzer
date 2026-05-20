#pragma once

#include <TopoDS_Shape.hxx>

#include <string>

TopoDS_Shape readStepFile(const std::string& path);
