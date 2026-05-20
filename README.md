# CAD Model Analyzer

基于 **C++17 + OpenCASCADE** 的 STEP 模型几何分析工具。项目读取 `.step/.stp` CAD 文件，解析为 OpenCASCADE 的 `TopoDS_Shape`，统计模型拓扑结构、曲线/曲面类型，计算包围盒、表面积、体积、质心，并做基础模型质量判断，最后输出结构化 JSON。

这个项目定位为 **三维 CAD 软件开发 / CAD 几何内核 / 工业软件 C++ 岗位的面试展示项目**，重点对应以下岗位要求：

- C++ 和数据结构基础。
- OpenCASCADE / ACIS / Parasolid 等几何内核理解。
- B-Rep 拓扑结构、曲线曲面和计算几何基础。
- CAD 数据转换、模型质量检查和工业数据结构化。
- CMake、MSVC、Windows DLL 依赖、脚本化测试等工程化能力。

---

## 30 秒项目介绍

> 我做了一个基于 C++17 和 OpenCASCADE 的 CAD 模型分析工具。它可以读取 STEP 文件，把模型转换成 `TopoDS_Shape`，然后遍历 B-Rep 拓扑结构，统计 Vertex、Edge、Face、Solid 等对象，识别 Edge 背后的曲线类型和 Face 背后的曲面类型，计算包围盒、面积、体积、质心，并检查自由边、非流形边等基础质量问题，最后输出 JSON 报告。这个项目主要用来展示我对 CAD 几何内核、B-Rep 数据结构、OpenCASCADE API 和 C++ 工程化的实践能力。

---

## 项目亮点

- **CAD 文件解析**：使用 OpenCASCADE 读取 STEP 格式模型。
- **B-Rep 拓扑分析**：统计 Vertex、Edge、Wire、Face、Shell、Solid、Compound。
- **几何类型识别**：识别直线、圆、椭圆、B 样条曲线，以及平面、圆柱面、球面、圆锥面、圆环面、B 样条曲面。
- **工程指标计算**：输出 bounding box、包围盒中心、对角线长度、表面积、体积、质心。
- **模型质量检查**：统计 free edge、non-manifold edge，判断是否为闭合实体候选模型。
- **结构化输出**：输出 metadata、topology、curves、surfaces、metrics、quality，便于接入 Web、数据库、报价系统或模型质检流程。
- **工程化脚本**：提供 Windows 下构建、运行和回归测试脚本，处理 OCCT/第三方 DLL 依赖路径。
- **可讲解性强**：模块边界清晰，适合面试时讲解 CAD 数据从文件到结构化报告的完整流程。

---

## 技术栈

- 语言：C++17
- CAD/几何内核：OpenCASCADE 8.0
- 构建系统：CMake + Ninja
- 编译环境：Visual Studio 2022 MSVC x64
- 平台：Windows
- 输出格式：JSON

---

## 功能清单

### 1. STEP 读取

读取 `.step` / `.stp` 文件，并转换为 OpenCASCADE 的 `TopoDS_Shape`。

对应模块：

- `src/StepReader.cpp`
- `include/StepReader.h`

### 2. 拓扑统计和模型质量检查

统计 B-Rep 模型中的基础拓扑对象：

- Vertex
- Edge
- Wire
- Face
- Shell
- Solid
- Compound

同时输出基础质量指标：

- `free_edge`：只被 0 或 1 个 Face 使用的边，常用于发现开放壳或未闭合模型。
- `non_manifold_edge`：被超过 2 个 Face 共享的边，常用于发现非流形拓扑。
- `euler_characteristic`：`V - E + F`，用于辅助理解拓扑复杂度。
- `closed_solid_candidate`：基于单实体、无自由边、无非流形边、正体积的闭合实体候选判断。

对应模块：

- `src/TopologyCounter.cpp`
- `include/TopologyCounter.h`

### 3. 几何分类

识别 Edge 对应曲线类型：

- Line
- Circle
- Ellipse
- BSplineCurve
- Other

识别 Face 对应曲面类型：

- Plane
- Cylinder
- Sphere
- Cone
- Torus
- BSplineSurface
- Other

对应模块：

- `src/GeometryClassifier.cpp`
- `include/GeometryClassifier.h`

### 4. 几何/质量属性计算

计算：

- Bounding box 最小/最大坐标
- Bounding box 长宽高 `dx/dy/dz`
- Bounding box 中心点
- Bounding box 对角线长度
- Surface area
- Volume
- Center of mass

对应模块：

- `src/ShapeMetrics.cpp`
- `include/ShapeMetrics.h`

### 5. JSON 报告输出

将分析结果序列化为 JSON，并写入指定输出文件。

对应模块：

- `src/JsonReport.cpp`
- `include/JsonReport.h`

---

## 架构概览

```text
STEP 文件
   ↓
StepReader / STEPControl_Reader
   ↓
TopoDS_Shape
   ↓
┌────────────────────┬──────────────────────┬────────────────┐
│ TopologyCounter    │ GeometryClassifier   │ ShapeMetrics   │
│ 拓扑统计 + 质检      │ 曲线/曲面分类          │ 尺寸/质量属性    │
└────────────────────┴──────────────────────┴────────────────┘
   ↓
JsonReport
   ↓
JSON 报告：metadata / topology / curves / surfaces / metrics / quality
```

更详细说明见：[`docs/architecture.md`](docs/architecture.md)

---

## 目录结构

```text
cad-model-analyzer/
  CMakeLists.txt
  README.md
  build-windows.bat          # Windows 构建脚本
  run-analyzer.bat           # 运行脚本，内置 OCCT DLL PATH
  test-screw.bat             # 最小回归测试
  include/
    StepReader.h
    TopologyCounter.h
    GeometryClassifier.h
    ShapeMetrics.h
    JsonReport.h
  src/
    main.cpp
    StepReader.cpp
    TopologyCounter.cpp
    GeometryClassifier.cpp
    ShapeMetrics.cpp
    JsonReport.cpp
  docs/
    architecture.md
    interview-demo.md
    roadmap.md
    demo-report.json
```

---

## 环境要求

当前项目使用本机已有 OpenCASCADE：

```text
D:\CodeProj\CoreEngine\occt-combined-release-no-pch\opencascade-8.0.0-vc14-64
```

构建脚本中已经配置：

- OCCT CMake 路径
- OCCT DLL 路径
- 第三方 DLL 路径
- Visual Studio 2022 Developer Command Prompt
- Ninja 路径

正常情况下不需要手动修改 `PATH`。

---

## 构建

在 Windows 环境执行：

```bat
D:\CodeProj\cad-model-analyzer\build-windows.bat
```

生成文件：

```text
D:\CodeProj\cad-model-analyzer\build\x64-release\cad_model_analyzer.exe
```

---

## 运行

推荐写法：

```bat
D:\CodeProj\cad-model-analyzer\run-analyzer.bat input.step -o output.json
```

也兼容旧写法：

```bat
D:\CodeProj\cad-model-analyzer\run-analyzer.bat input.step output.json
```

查看帮助和版本：

```bat
D:\CodeProj\cad-model-analyzer\run-analyzer.bat --help
D:\CodeProj\cad-model-analyzer\run-analyzer.bat --version
```

示例：

```bat
D:\CodeProj\cad-model-analyzer\run-analyzer.bat D:\CodeProj\CoreEngine\occt-combined-release-no-pch\opencascade-8.0.0-vc14-64\data\step\screw.step D:\CodeProj\cad-model-analyzer\output\sample-output.json
```

---

## 测试

运行最小回归测试：

```bat
D:\CodeProj\cad-model-analyzer\test-screw.bat
```

测试内容：

- `--help` 能正常运行
- `--version` 能正常运行
- `-o output.json` 写法可用
- 旧版 `input.step output.json` 写法可用
- `screw.step` 的关键拓扑/几何统计符合预期
- metadata、metrics、quality 字段存在
- free edge / non-manifold edge / closed solid candidate 等质检字段符合预期

成功输出示例：

```text
OK: screw.step report matches expected counts, metrics, and CLI options
```

---

## 输出示例

完整示例见：[`docs/demo-report.json`](docs/demo-report.json)

节选：

```json
{
  "metadata": {
    "analyzer": "cad_model_analyzer",
    "version": "0.2.0",
    "input_file": "samples/screw.step",
    "analysis_time_ms": 20
  },
  "topology": {
    "vertex": 88,
    "edge": 44,
    "wire": 10,
    "face": 10,
    "shell": 1,
    "solid": 1,
    "compound": 0,
    "free_edge": 0,
    "non_manifold_edge": 0,
    "euler_characteristic": 54
  },
  "curves": {
    "line": 10,
    "circle": 20,
    "ellipse": 0,
    "bspline_curve": 14,
    "other": 0
  },
  "surfaces": {
    "plane": 4,
    "cylinder": 1,
    "sphere": 0,
    "cone": 2,
    "torus": 3,
    "bspline_surface": 0,
    "other": 0
  },
  "quality": {
    "closed_solid_candidate": true,
    "has_free_edges": false,
    "has_non_manifold_edges": false,
    "has_positive_volume": true
  }
}
```

---

## 面试要求对应关系

### C++ 和数据结构

- 使用结构体组织 `TopologyStats`、`CurveStats`、`SurfaceStats`、`ShapeMetrics`。
- 使用 CMake 管理多源文件模块。
- 使用 `std::string`、异常、文件输出、命令行参数解析和计时。
- 通过 smoke test 验证 CLI 行为和 JSON 字段。

### OpenCASCADE / 几何内核

- `STEPControl_Reader`：STEP 文件导入。
- `TopoDS_Shape`：B-Rep 模型入口。
- `TopExp_Explorer`：按拓扑类型遍历。
- `TopExp::MapShapesAndAncestors`：建立 Edge 到 Face 的邻接关系，用于自由边和非流形边检查。
- `BRep_Tool::Curve` / `BRep_Tool::Surface`：从拓扑对象取到底层几何对象。
- `BRepBndLib` / `BRepGProp`：计算包围盒和质量属性。

### B-Rep 和计算几何基础

- 区分拓扑对象：Vertex / Edge / Wire / Face / Shell / Solid。
- 区分几何对象：Line / Circle / BSplineCurve / Plane / Cylinder / Torus。
- 输出 `V - E + F` 欧拉特征数，辅助解释拓扑复杂度。
- 使用边-面邻接关系判断开放边和非流形边。

### 工业软件工程化

- Windows + MSVC + CMake + Ninja 构建。
- 处理 OCCT 多层 DLL 依赖。
- 输出 JSON，便于接入 Web、数据库、批处理、报价或质检系统。
- 提供 `test-screw.bat` 做最小回归测试。

---

## 面试时可以重点讲什么

### 1. 为什么做这个项目

CAD/CAE 软件不是普通文本或图片处理，核心数据结构是 B-Rep 几何拓扑。这个项目用 OpenCASCADE 读取 STEP 文件，把复杂 CAD 模型转换成可分析、可存储、可展示的结构化数据。

### 2. 技术难点

- 理解 `TopoDS_Shape` 只是一个通用形状入口，需要通过 `TopExp_Explorer` 遍历不同拓扑类型。
- Edge 和 Face 本身不是几何，需要通过 `BRep_Tool::Curve` / `BRep_Tool::Surface` 取到底层几何对象。
- 开放边和非流形边不是单纯计数，需要建立 Edge 到 Face 的邻接关系。
- 面积、体积、质心不是简单遍历，需要调用 `BRepGProp` 的属性计算接口。
- Windows 下 OCCT 程序运行依赖多层 DLL，需要处理运行时路径和间接依赖。

### 3. 项目价值

这个工具可以作为 CAD 数据服务的基础模块，后续可以扩展为：

- 模型质量检查
- 零件复杂度评估
- 自动报价辅助
- Web 上传分析服务
- 批量模型统计
- CAD 数据入库

### 4. 后续扩展方向

如果继续迭代，可以增加：

- 模型质量检查：空模型、多 Solid、Shell-only、开放边详情、非流形边详情。
- 批量分析目录并输出 CSV。
- 支持 IGES / BREP / STL。
- 增加 Web 或桌面 UI。
- 对接数据库，保存模型分析结果。
- 输出可视化预览图。

---

## 面试讲解资料

- 架构说明：[`docs/architecture.md`](docs/architecture.md)
- 面试讲解稿：[`docs/interview-demo.md`](docs/interview-demo.md)
- Roadmap：[`docs/roadmap.md`](docs/roadmap.md)
- 示例输出：[`docs/demo-report.json`](docs/demo-report.json)
