# CAD Model Analyzer 架构说明

本文档说明 CAD Model Analyzer 的模块设计、数据流和关键 OpenCASCADE API。目标是让代码阅读者快速理解：一个 STEP 文件如何被转换成结构化 JSON 分析报告，以及如何做基础模型质量判断。

---

## 总体目标

项目输入是工业 CAD 场景常见的 STEP 文件，输出是适合程序消费的 JSON 报告。

输入：

```text
part.step / part.stp
```

输出：

```json
{
  "metadata": {},
  "topology": {},
  "curves": {},
  "surfaces": {},
  "metrics": {},
  "quality": {}
}
```

核心目标不是渲染模型，而是做 **CAD 数据解析、B-Rep 分析和基础模型质量检查**。

---

## 高层数据流

```text
┌─────────────┐
│ STEP 文件   │
└──────┬──────┘
       │
       ▼
┌────────────────────┐
│ StepReader          │
│ STEPControl_Reader  │
└──────┬─────────────┘
       │ TopoDS_Shape
       ▼
┌───────────────────────────────────────────────────────┐
│ Analysis Layer                                         │
│                                                       │
│  ┌─────────────────┐  ┌──────────────────────┐        │
│  │ TopologyCounter │  │ GeometryClassifier   │        │
│  │ 拓扑统计 + 质检  │  │ 曲线/曲面分类          │        │
│  └─────────────────┘  └──────────────────────┘        │
│                                                       │
│  ┌─────────────────┐                                  │
│  │ ShapeMetrics    │                                  │
│  │ 尺寸/质量属性     │                                  │
│  └─────────────────┘                                  │
└──────┬────────────────────────────────────────────────┘
       │ Stats + Metrics + Quality
       ▼
┌────────────────────┐
│ JsonReport          │
│ JSON 序列化/写文件   │
└──────┬─────────────┘
       │
       ▼
┌─────────────┐
│ report.json │
└─────────────┘
```

---

## 模块划分

### 1. `main.cpp`

职责：程序入口和流程编排。

主要工作：

1. 解析命令行参数。
2. 调用 `readStepFile` 读取 STEP。
3. 调用各分析模块生成统计结果。
4. 记录分析耗时 `analysis_time_ms`。
5. 调用 `buildJsonReport` 生成 JSON。
6. 写入输出文件并打印到控制台。

主流程伪代码：

```cpp
auto start = std::chrono::steady_clock::now();
TopoDS_Shape shape = readStepFile(inputFile);
TopologyStats topology = countTopology(shape);
CurveStats curves = classifyCurves(shape);
SurfaceStats surfaces = classifySurfaces(shape);
ShapeMetrics metrics = measureShape(shape);
auto elapsedMs = ...;
std::string report = buildJsonReport(inputFile, version, elapsedMs, topology, curves, surfaces, metrics);
writeTextFile(outputFile, report);
```

设计说明：

- `main.cpp` 不直接写分析细节，只做编排。
- 各分析模块可以独立扩展和测试。
- metadata 输出版本和耗时，便于后续做批量处理和性能对比。

---

### 2. `StepReader`

文件：

- `include/StepReader.h`
- `src/StepReader.cpp`

职责：读取 STEP 文件并返回 `TopoDS_Shape`。

关键 OpenCASCADE API：

- `STEPControl_Reader`
- `ReadFile`
- `TransferRoots`
- `OneShape`

概念说明：

- STEP 是中性 CAD 文件格式。
- OpenCASCADE 读取后会将模型转换为内部 B-Rep 表达。
- `TopoDS_Shape` 是后续拓扑遍历和几何分析的入口。

---

### 3. `TopologyCounter`

文件：

- `include/TopologyCounter.h`
- `src/TopologyCounter.cpp`

职责：统计 B-Rep 拓扑对象数量，并做基础模型质量检查。

输出结构：

```cpp
struct TopologyStats {
    int vertex;
    int edge;
    int wire;
    int face;
    int shell;
    int solid;
    int compound;
    int freeEdge;
    int nonManifoldEdge;
    int eulerCharacteristic;
};
```

关键 OpenCASCADE API：

- `TopExp_Explorer`
- `TopAbs_VERTEX`
- `TopAbs_EDGE`
- `TopAbs_FACE`
- `TopExp::MapShapesAndAncestors`
- `TopTools_IndexedDataMapOfShapeListOfShape`

拓扑计数实现思路：

```cpp
for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
    ++count;
}
```

自由边 / 非流形边检查思路：

```cpp
TopTools_IndexedDataMapOfShapeListOfShape edgeFaceMap;
TopExp::MapShapesAndAncestors(shape, TopAbs_EDGE, TopAbs_FACE, edgeFaceMap);
for each edge:
    adjacentFaceCount = edgeFaceMap[edge].Extent();
    if adjacentFaceCount < 2: free edge
    if adjacentFaceCount > 2: non-manifold edge
```

技术说明：

> OpenCASCADE 中 CAD 模型是拓扑和几何组合形成的。这里统计的是拓扑层面的对象，例如 Face、Edge 和 Vertex，而不是三角网格顶点。进一步通过 Edge 到 Face 的邻接关系，可以判断开放边和非流形边，这比单纯读文件更接近 CAD 模型质量检查。

---

### 4. `GeometryClassifier`

文件：

- `include/GeometryClassifier.h`
- `src/GeometryClassifier.cpp`

职责：识别曲线和曲面类型。

#### Edge 曲线分类

输出结构：

```cpp
struct CurveStats {
    int line;
    int circle;
    int ellipse;
    int bsplineCurve;
    int other;
};
```

关键 OpenCASCADE API：

- `BRep_Tool::Curve`
- `Geom_Curve`
- `Geom_Line`
- `Geom_Circle`
- `Geom_Ellipse`
- `Geom_BSplineCurve`
- `IsKind(STANDARD_TYPE(...))`

流程：

```text
TopoDS_Edge
   ↓ BRep_Tool::Curve
Geom_Curve
   ↓ IsKind
Line / Circle / Ellipse / BSplineCurve / Other
```

#### Face 曲面分类

输出结构：

```cpp
struct SurfaceStats {
    int plane;
    int cylinder;
    int sphere;
    int cone;
    int torus;
    int bsplineSurface;
    int other;
};
```

关键 OpenCASCADE API：

- `BRep_Tool::Surface`
- `Geom_Surface`
- `Geom_Plane`
- `Geom_CylindricalSurface`
- `Geom_SphericalSurface`
- `Geom_ConicalSurface`
- `Geom_ToroidalSurface`
- `Geom_BSplineSurface`

技术说明：

> Edge/Face 是拓扑对象，Line/Circle/Plane/Cylinder 是几何对象。项目中通过 `BRep_Tool` 从拓扑对象取到底层几何对象，再判断具体类型。

---

### 5. `ShapeMetrics`

文件：

- `include/ShapeMetrics.h`
- `src/ShapeMetrics.cpp`

职责：计算几何尺寸和质量属性。

输出结构：

```cpp
struct ShapeMetrics {
    BoundingBoxMetrics boundingBox;
    PointMetrics centerOfMass;
    double surfaceArea;
    double volume;
};
```

#### Bounding box

关键 API：

- `Bnd_Box`
- `BRepBndLib::Add`

输出：

- `xmin/ymin/zmin`
- `xmax/ymax/zmax`
- `dx/dy/dz`
- `center.x/y/z`
- `diagonal`

计算逻辑：

```cpp
dx = xmax - xmin;
dy = ymax - ymin;
dz = zmax - zmin;
centerX = (xmin + xmax) / 2.0;
diagonal = sqrt(dx * dx + dy * dy + dz * dz);
```

#### Surface area / Volume / Center of mass

关键 API：

- `BRepGProp::SurfaceProperties`
- `BRepGProp::VolumeProperties`
- `GProp_GProps::Mass`
- `GProp_GProps::CentreOfMass`

说明：

- 对 SurfaceProperties，`Mass()` 表示表面积。
- 对 VolumeProperties，`Mass()` 表示体积。
- `CentreOfMass()` 返回质心坐标。

技术说明：

> 这些指标可以用于工业场景中的模型尺寸分析、报价辅助、重量估算、复杂度评估或质量检查。

---

### 6. `JsonReport`

文件：

- `include/JsonReport.h`
- `src/JsonReport.cpp`

职责：将分析结果序列化为 JSON。

输出结构：

```json
{
  "metadata": {
    "analyzer": "cad_model_analyzer",
    "version": "0.3.0",
    "input_file": "samples/screw.step",
    "analysis_time_ms": 20
  },
  "topology": {
    "free_edge": 0,
    "non_manifold_edge": 0,
    "euler_characteristic": 54
  },
  "quality": {
    "closed_solid_candidate": true,
    "has_free_edges": false,
    "has_non_manifold_edges": false,
    "has_positive_volume": true
  }
}
```

设计取舍：

- 当前没有引入第三方 JSON 库，使用 `std::ostringstream` 手动生成 JSON。
- 好处是项目依赖简单，适合小型命令行工具和轻量级工程模块。
- 后续如果扩展复杂字段，可以替换为 `nlohmann/json`。

---

## 命令行接口设计

支持：

```bat
cad_model_analyzer.exe input.step output.json
cad_model_analyzer.exe input.step -o output.json
cad_model_analyzer.exe --help
cad_model_analyzer.exe --version
```

设计考虑：

- 保留简单旧写法，方便快速测试。
- 增加 `-o`，更接近常见 CLI 工具风格。
- `--help` / `--version` 方便演示、排障和脚本检查。

---

## 构建和运行设计

### CMake

`CMakeLists.txt` 中显式链接 OpenCASCADE 相关模块：

- `TKernel`
- `TKMath`
- `TKG2d`
- `TKG3d`
- `TKBRep`
- `TKGeomBase`
- `TKGeomAlgo`
- `TKTopAlgo`
- `TKPrim`
- `TKBO`
- `TKShHealing`
- `TKXSBase`
- `TKDESTEP`

### Windows 脚本

- `build-windows.bat`：进入 MSVC x64 环境，配置 CMake，执行构建。
- `run-analyzer.bat`：设置 OCCT 和第三方 DLL 的 PATH，然后运行 exe。
- `test-screw.bat`：运行 smoke test 和最小回归测试。

### 为什么需要脚本处理 DLL？

Windows 下 exe 加载 DLL 依赖时，需要在可执行文件目录、系统目录或 PATH 中找到相关 DLL。OpenCASCADE 本身依赖多个 OCCT DLL 和第三方 DLL，所以只编译成功还不够，运行时路径也必须正确。

---

## 当前测试策略

测试脚本：

```bat
D:\CodeProj\cad-model-analyzer\test-screw.bat
```

测试样例：

```text
D:\CodeProj\CoreEngine\occt-combined-release-no-pch\opencascade-8.0.0-vc14-64\data\step\screw.step
```

验证内容：

- 程序能正常启动。
- `--help` 可用。
- `--version` 可用。
- `-o` 参数可用。
- 旧版输出参数可用。
- 关键拓扑数量符合预期。
- 关键曲线/曲面统计符合预期。
- `metrics` 字段存在，且包围盒、面积、体积、质心有效。
- `metadata` 字段存在，版本和分析耗时有效。
- `quality` 字段存在，自由边、非流形边、闭合实体候选判断符合预期。

---

## 后续可扩展方向

详细路线见：[`docs/roadmap.md`](roadmap.md)

### 1. 模型质量检查

可增加：

- 是否为空模型
- 是否多 Solid
- 是否只有 Shell 没有 Solid
- 开放边详情
- 非流形边详情
- 体积是否为 0

### 2. 批量分析

已支持目录输入：

```bat
cad_model_analyzer.exe --batch models -o reports
```

输出：

- `reports/summary.csv`：汇总每个模型的状态、实体数、面数、边数、自由边、非流形边、体积、面积、耗时和 JSON 文件名。
- `reports/<model>.json`：每个模型的完整分析报告。

这部分把单模型分析扩展成批处理能力，便于后续接入模型库、报价系统或质检流水线。

### 3. 更多格式支持

可扩展：

- IGES
- BREP
- STL

### 4. Web/GUI 展示

将当前命令行工具封装为：

- 本地桌面工具
- Web 上传分析服务
- 后端 CAD 分析 API

---

## 代码阅读推荐顺序

1. `README.md`：理解项目目标和能力边界。
2. `src/main.cpp`：理解主流程。
3. `src/StepReader.cpp`：理解 STEP 读取。
4. `src/TopologyCounter.cpp`：理解拓扑遍历和基础质检。
5. `src/GeometryClassifier.cpp`：理解拓扑到几何的转换。
6. `src/ShapeMetrics.cpp`：理解工程属性计算。
7. `src/JsonReport.cpp`：理解结果输出。
8. `test-screw.bat`：理解验证方式。

---

## 设计边界

当前项目有意保持简单：

- 不做 3D 渲染。
- 不做 GUI。
- 不引入数据库。
- 不引入第三方 JSON 库。
- 不处理所有 CAD 模型质量问题。

原因：

> 当前目标是实现一个轻量级 CAD 数据分析模块，重点覆盖 CAD 文件解析、B-Rep 分析、模型质量检查和 C++ 工程化能力，而不是做完整商业 CAD 软件。
