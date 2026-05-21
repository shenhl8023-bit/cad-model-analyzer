# CAD Model Analyzer

基于 **C++17 + OpenCASCADE** 的 STEP 模型几何分析工具。项目读取 `.step/.stp` CAD 文件，解析为 OpenCASCADE 的 `TopoDS_Shape`，统计模型拓扑结构、曲线/曲面类型，计算包围盒、表面积、体积、质心，并输出模型质量状态和复杂度等级，最后生成结构化 JSON/CSV。

这个项目面向 **CAD 数据解析、模型质量检查和工程数据结构化** 场景，重点覆盖以下技术能力：

- C++ 和数据结构基础。
- OpenCASCADE / ACIS / Parasolid 等几何内核理解。
- B-Rep 拓扑结构、曲线曲面和计算几何基础。
- CAD 数据转换、模型质量检查和工业数据结构化。
- CMake、MSVC、Windows DLL 依赖、脚本化测试等工程化能力。

---

## 项目概述

CAD Model Analyzer 是一个基于 C++17 和 OpenCASCADE 的 CAD 模型分析工具。它读取 STEP 文件并转换为 `TopoDS_Shape`，遍历 B-Rep 拓扑结构，统计 Vertex、Edge、Face、Solid 等对象，识别 Edge 背后的曲线类型和 Face 背后的曲面类型，计算包围盒、面积、体积、质心，并检查自由边、非流形边等基础质量问题，最后输出 JSON/CSV 报告。

---

## 项目亮点

- **CAD 文件解析**：使用 OpenCASCADE 读取 STEP 格式模型。
- **B-Rep 拓扑分析**：统计 Vertex、Edge、Wire、Face、Shell、Solid、Compound。
- **几何类型识别**：识别直线、圆、椭圆、B 样条曲线，以及平面、圆柱面、球面、圆锥面、圆环面、B 样条曲面。
- **工程指标计算**：输出 bounding box、包围盒中心、对角线长度、表面积、体积、质心。
- **模型质量检查**：统计 free edge、non-manifold edge，判断是否为闭合实体候选模型。
- **结构化输出**：输出 metadata、topology、curves、surfaces、metrics、quality，包含 `status`、`issue_count`、`complexity_level` 等字段，便于接入 Web、数据库、报价系统或模型质检流程。
- **轻量 Web 分析界面**：提供本地 Web 页面上传 STEP/STP 文件，调用现有分析器并展示质量状态、复杂度、关键拓扑指标和完整 JSON。
- **工程化脚本**：提供 Windows 下构建、运行和回归测试脚本，处理 OCCT/第三方 DLL 依赖路径。
- **文档化说明**：模块边界清晰，便于理解 CAD 数据从文件到结构化报告的完整流程。

---

## 技术栈

- 语言：C++17
- CAD/几何内核：OpenCASCADE 8.0
- 构建系统：CMake + Ninja
- 编译环境：Visual Studio 2022 MSVC x64
- 平台：Windows
- 输出格式：JSON / CSV
- 可选 Web 层：Node.js + Fastify

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
- `status`：综合质量状态，例如 `ok`、`open_shell`、`non_manifold`、`multi_solid`、`shell_only`、`empty`。
- `issue_count`：基础质量问题数量，用于批量筛选异常模型。
- `complexity_level`：基于拓扑数量的复杂度等级，当前分为 `low`、`medium`、`high`。

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
    QualityAssessment.cpp
    JsonReport.cpp
  tests/
    quality_assessment_tests.cpp
  web/
    server.js
    smoke-test.js
    public/
      index.html
      app.js
      style.css
  docs/
    architecture.md
    technical-walkthrough.md
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

`--help` 会列出三类信息：

- 命令格式：单文件分析、`-o` 指定输出、批量分析。
- 报告内容：`metadata`、`topology`、`curves`、`surfaces`、`metrics`、`quality`。
- 质量状态：`ok`、`empty`、`open_shell`、`non_manifold`、`multi_solid`、`shell_only`、`invalid_volume`。

批量分析目录并输出每个模型的 JSON 和 CSV 汇总：

```bat
D:\CodeProj\cad-model-analyzer\run-analyzer.bat --batch D:\models\step-files -o D:\models\cad-report
```

批量模式会生成：

- `summary.csv`：每个 STEP 文件一行，包含整体处理状态、模型质量状态、复杂度等级、问题数、实体数、面数、边数、自由边、流形边、非流形边、最大边-面邻接数、体积、面积、耗时和报告文件名。
- `<model-name>.json`：每个模型的完整 JSON 分析报告。

示例：

```bat
D:\CodeProj\cad-model-analyzer\run-analyzer.bat D:\CodeProj\CoreEngine\occt-combined-release-no-pch\opencascade-8.0.0-vc14-64\data\step\screw.step D:\CodeProj\cad-model-analyzer\output\sample-output.json
```

---

## Web 分析界面

Web 层是对现有命令行分析器的轻量封装：浏览器上传 STEP/STP 文件，Node.js 服务把文件保存到本地临时目录，调用 `cad_model_analyzer.exe` 生成 JSON，再把质量状态、复杂度、关键拓扑指标和完整 JSON 返回给前端展示。

安装依赖：

```bat
cd /d D:\CodeProj\cad-model-analyzer
npm install
```

启动前先确保 C++ 分析器已经构建完成：

```bat
D:\CodeProj\cad-model-analyzer\build-windows.bat
```

启动 Web 服务：

```bat
npm run web
```

默认访问地址：

```text
http://127.0.0.1:3307
```

可配置环境变量见 `.env.example`：

- `CAD_WEB_HOST`：监听地址，默认 `127.0.0.1`。
- `CAD_WEB_PORT`：监听端口，默认 `3307`。
- `CAD_ANALYZER_EXE`：`cad_model_analyzer.exe` 路径。
- `CAD_UPLOAD_DIR`：上传 STEP/STP 文件保存目录。
- `CAD_REPORT_DIR`：分析报告保存目录。
- `CAD_MAX_UPLOAD_MB`：最大上传体积，默认 50 MB。

Web smoke test：

```bat
npm run web:smoke
```

Web 层当前定位是本地交互分析界面，不包含账号、权限、数据库或大型 CAD 预览器。实际部署时建议放在内网或受控环境中，通过反向代理限制访问范围。

---

## 测试

运行最小回归测试：

```bat
D:\CodeProj\cad-model-analyzer\test-screw.bat
```

运行 Web 层 smoke test：

```bat
npm run web:smoke
```

测试内容：

- `--help` 能正常运行
- `--version` 能正常运行
- `-o output.json` 写法可用
- 旧版 `input.step output.json` 写法可用
- `--batch input_dir -o output_dir` 批量分析可用
- Web 服务健康检查、首页和 STEP 上传分析链路可用
- `screw.step` 的关键拓扑/几何统计符合预期
- metadata、metrics、quality 字段存在
- free edge / non-manifold edge / closed solid candidate / status / complexity level 等质检字段符合预期

成功输出示例：

```text
OK: screw.step report matches expected counts, metrics, and CLI options
```

---

## 输出示例

完整示例见：[`docs/demo-report.json`](docs/demo-report.json)

批量输出示例：

- [`docs/demo-summary.csv`](docs/demo-summary.csv)
- [`docs/demo-batch-screw.json`](docs/demo-batch-screw.json)

节选：

```json
{
  "metadata": {
    "analyzer": "cad_model_analyzer",
    "version": "0.5.0",
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
    "isolated_edge": 0,
    "free_edge": 0,
    "manifold_edge": 22,
    "non_manifold_edge": 0,
    "max_edge_face_adjacency": 2,
    "edge_face_adjacency": {
      "isolated": 0,
      "boundary": 0,
      "manifold": 22,
      "non_manifold": 0,
      "max_faces_per_edge": 2
    },
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
    "has_positive_volume": true,
    "is_empty": false,
    "multi_solid": false,
    "shell_only": false,
    "issue_count": 0,
    "status": "ok",
    "complexity_level": "low"
  }
}
```

---

## 输出字段说明

### JSON 主要字段

- `metadata`：分析器名称、版本、输入文件路径和分析耗时。
- `topology`：B-Rep 拓扑统计，包括点、边、线框、面、壳、实体、复合体，以及边-面邻接关系。
- `curves`：Edge 背后的曲线类型统计，例如直线、圆、椭圆和 B 样条曲线。
- `surfaces`：Face 背后的曲面类型统计，例如平面、圆柱面、圆锥面、圆环面和 B 样条曲面。
- `metrics`：包围盒、表面积、体积和质心等几何/质量属性。
- `quality`：面向批量筛选的质量判断结果。

### topology 重点字段

- `free_edge`：边界边数量，即没有被两个 Face 正常共享的边。数量大于 0 时通常说明模型存在开放边界。
- `manifold_edge`：被两个 Face 共享的正常流形边数量。
- `non_manifold_edge`：被超过两个 Face 共享的边。数量大于 0 时通常说明拓扑关系异常。
- `max_edge_face_adjacency`：单条边最多被多少个 Face 共享，用于定位非流形复杂度。
- `euler_characteristic`：`V - E + F`，用于辅助观察拓扑结构。

### quality 重点字段

- `closed_solid_candidate`：基于单实体、无自由边、无非流形边和正体积的闭合实体候选判断。
- `status`：综合状态。常见值包括：
  - `ok`：未发现基础拓扑/体积异常。
  - `empty`：未识别到有效拓扑对象。
  - `open_shell`：存在开放边界。
  - `non_manifold`：存在非流形边。
  - `multi_solid`：存在多个 Solid。
  - `shell_only`：存在 Shell/Face，但没有 Solid。
  - `invalid_volume`：存在 Solid，但体积不是正数。
- `issue_count`：基础问题数量，便于排序和筛选。
- `complexity_level`：基于拓扑规模的复杂度等级，当前为 `low`、`medium`、`high`。

### CSV 汇总字段

批量模式的 `summary.csv` 面向快速筛选，字段含义如下：

- `file`：输入 STEP/STP 文件名。
- `status`：文件处理状态，`ok` 表示读取和分析成功，`error` 表示该文件失败。
- `quality_status`：模型质量状态，对应 JSON 中的 `quality.status`。
- `complexity_level`：复杂度等级。
- `issue_count`：基础问题数量。
- `solid` / `face` / `edge`：主要拓扑规模。
- `free_edge` / `manifold_edge` / `non_manifold_edge` / `max_edge_face_adjacency`：边-面邻接质量指标。
- `volume` / `surface_area`：体积和表面积。
- `analysis_time_ms`：单个模型分析耗时。
- `report`：对应的详细 JSON 文件名。

---

## 技术能力覆盖

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
- 输出综合质量状态和复杂度等级，让批量处理结果可以直接筛选可用模型和异常模型。

### 工业软件工程化

- Windows + MSVC + CMake + Ninja 构建。
- 处理 OCCT 多层 DLL 依赖。
- 输出 JSON，便于接入 Web、数据库、批处理、报价或质检系统。
- 提供 `test-screw.bat` 做最小回归测试。

---

### 1. 为什么做这个项目

CAD/CAE 软件不是普通文本或图片处理，核心数据结构是 B-Rep 几何拓扑。这个项目用 OpenCASCADE 读取 STEP 文件，把复杂 CAD 模型转换成可分析、可存储、可查询的结构化数据。

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

## 项目文档

- 能力覆盖说明：[`docs/capability-map.md`](docs/capability-map.md)
- 架构说明：[`docs/architecture.md`](docs/architecture.md)
- 技术说明：[`docs/technical-walkthrough.md`](docs/technical-walkthrough.md)
- Roadmap：[`docs/roadmap.md`](docs/roadmap.md)
- 示例输出：[`docs/demo-report.json`](docs/demo-report.json)
