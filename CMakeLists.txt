cmake_minimum_required(VERSION 3.20)

project(cad_model_analyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenCASCADE REQUIRED)

# 本项目只用 STEP 读取和 BREP 分析。OCCT 的 CMake target 会传递不少 DLL，
# 所以下面关闭自动 DLL 拷贝，并在 build-windows.bat / run-analyzer.bat 里显式配置 PATH。

add_executable(cad_model_analyzer
    src/main.cpp
    src/Analysis.cpp
    src/StepReader.cpp
    src/TopologyCounter.cpp
    src/GeometryClassifier.cpp
    src/ShapeMetrics.cpp
    src/QualityAssessment.cpp
    src/JsonReport.cpp
)

add_executable(quality_assessment_tests
    tests/quality_assessment_tests.cpp
    src/QualityAssessment.cpp
)

target_include_directories(cad_model_analyzer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCASCADE_INCLUDE_DIR}
)

target_include_directories(quality_assessment_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCASCADE_INCLUDE_DIR}
)

target_link_libraries(cad_model_analyzer PRIVATE
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKBRep
    TKGeomBase
    TKGeomAlgo
    TKTopAlgo
    TKPrim
    TKBO
    TKShHealing
    TKXSBase
    TKDESTEP
)

if (WIN32)
    set(OCCT_BIN_DIR "${OpenCASCADE_BINARY_DIR}")
    add_custom_command(TARGET cad_model_analyzer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OCCT_BIN_DIR}/TKernel.dll"
            "${OCCT_BIN_DIR}/TKMath.dll"
            "${OCCT_BIN_DIR}/TKG2d.dll"
            "${OCCT_BIN_DIR}/TKG3d.dll"
            "${OCCT_BIN_DIR}/TKBRep.dll"
            "${OCCT_BIN_DIR}/TKGeomBase.dll"
            "${OCCT_BIN_DIR}/TKGeomAlgo.dll"
            "${OCCT_BIN_DIR}/TKTopAlgo.dll"
            "${OCCT_BIN_DIR}/TKPrim.dll"
            "${OCCT_BIN_DIR}/TKBO.dll"
            "${OCCT_BIN_DIR}/TKShHealing.dll"
            "${OCCT_BIN_DIR}/TKXSBase.dll"
            "${OCCT_BIN_DIR}/TKDE.dll"
            "${OCCT_BIN_DIR}/TKDESTEP.dll"
            $<TARGET_FILE_DIR:cad_model_analyzer>
    )
endif()
