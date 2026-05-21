# CAD Model Analyzer 技术说明

本文档面向代码维护和二次开发，说明项目的处理流程、模块边界、关键 OpenCASCADE API、输出格式和后续扩展点。

---

## 1. 项目定位

CAD Model Analyzer 是一个命令行 CAD 数据分析工具，当前重点处理 STEP/STP 文件。它不负责建模和渲染，而是把 CAD 文件转换成可程序消费的结构化分析结果。

核心能力：

- 读取 STEP/STP 文件并转换为 OpenCASCADE 的 `TopoDS_Shape`。
- 遍历 B-Rep 拓扑对象，统计 Vertex、Edge、Wire、Face、Shell、Solid、Compound。
- 识别 Edge 对应的曲线类型和 Face 对应的曲面类型。
- 计算包围盒、表面积、体积、质心等工程属性。
- 检查自由边、非流形边和闭合实体候选状态。
- 输出 JSON 报告；批量模式下同时输出 CSV 汇总。

适合的集成场景：

- CAD 模型入库前的基础检查。
- STEP 文件批量统计和资产索引。
- 模型质检、报价、重量估算或复杂度评估的前置数据提取。
- Web/API 服务中的 CAD 分析后端模块。

---

## 2. 高层处理流程

```text
STEP/STP 文件
   ↓
StepReader
   ↓
TopoDS_Shape
   ↓
Analysis Layer
   ├─ TopologyCounter     拓扑统计和基础质量检查
   ├─ GeometryClassifier  曲线/曲面类型识别
   └─ ShapeMetrics        包围盒、面积、体积、质心
   ↓
JsonReport
   ↓
JSON 报告 / CSV 汇总
```

主程序只负责编排流程：解析参数、读取模型、调用分析模块、记录耗时、生成报告、写入输出文件。

---

## 3. 模块说明

### 3.1 `StepReader`

文件：

- `include/StepReader.h`
- `src/StepReader.cpp`

职责：读取 STEP/STP 文件，并返回 OpenCASCADE 内部的 `TopoDS_Shape`。

关键 API：

- `STEPControl_Reader`
- `ReadFile`
- `TransferRoots`
- `OneShape`

实现要点：

- STEP 是中性 CAD 交换格式，读取后需要转换成 OpenCASCADE 的 B-Rep 表达。
- `TopoDS_Shape` 是后续拓扑遍历、几何分类和属性计算的统一入口。
- 读取失败应抛出明确异常，避免后续模块处理空模型或无效模型。

### 3.2 `TopologyCounter`

文件：

- `include/TopologyCounter.h`
- `src/TopologyCounter.cpp`

职责：统计基础拓扑对象，并输出基础质量指标。

输出字段包括：

- `vertex`
- `edge`
- `wire`
- `face`
- `shell`
- `solid`
- `compound`
- `isolated_edge`
- `free_edge`
- `manifold_edge`
- `non_manifold_edge`
- `max_edge_face_adjacency`
- `edge_face_adjacency`
- `euler_characteristic`

关键 API：

- `TopExp_Explorer`
- `TopAbs_VERTEX` / `TopAbs_EDGE` / `TopAbs_FACE` 等拓扑类型
- `TopExp::MapShapesAndAncestors`
- `TopTools_IndexedDataMapOfShapeListOfShape`

自由边和非流形边的判断逻辑：

```text
建立 Edge -> Face 邻接关系
遍历每条 Edge：
  adjacentFaceCount == 0 => isolated_edge + free_edge
  adjacentFaceCount == 1 => free_edge
  adjacentFaceCount == 2 => manifold_edge
  adjacentFaceCount > 2  => non_manifold_edge
  同时记录 max_edge_face_adjacency
```

说明：

- 拓扑对象描述连接关系，不等同于网格顶点或三角面。
- 自由边通常意味着开放壳、缺面或边界边。
- 非流形边通常意味着模型拓扑不适合直接用于某些后续处理，例如实体布尔、网格划分或加工分析。

### 3.3 `GeometryClassifier`

文件：

- `include/GeometryClassifier.h`
- `src/GeometryClassifier.cpp`

职责：识别 Edge 底层曲线类型和 Face 底层曲面类型。

曲线类型：

- Line
- Circle
- Ellipse
- BSplineCurve
- Other

曲面类型：

- Plane
- Cylinder
- Sphere
- Cone
- Torus
- BSplineSurface
- Other

关键 API：

- `BRep_Tool::Curve`
- `BRep_Tool::Surface`
- `IsKind(STANDARD_TYPE(...))`

实现要点：

- Edge 和 Face 是拓扑对象，本身不直接说明几何类型。
- 需要通过 `BRep_Tool` 取得底层 `Geom_Curve` 或 `Geom_Surface`。
- 类型识别结果适合用于模型复杂度评估、规则检查和后续统计分析。

### 3.4 `ShapeMetrics`

文件：

- `include/ShapeMetrics.h`
- `src/ShapeMetrics.cpp`

职责：计算模型尺寸和质量属性。

输出指标：

- bounding box 最小/最大坐标。
- bounding box 尺寸 `dx / dy / dz`。
- bounding box 中心点。
- bounding box 空间对角线长度。
- 表面积。
- 体积。
- 质心。

关键 API：

- `Bnd_Box`
- `BRepBndLib::Add`
- `BRepGProp::SurfaceProperties`
- `BRepGProp::VolumeProperties`
- `GProp_GProps`

实现要点：

- `SurfaceProperties(...).Mass()` 表示表面积。
- `VolumeProperties(...).Mass()` 表示体积。
- `CentreOfMass()` 返回质心坐标。
- 对非封闭模型，体积和质心可能没有业务意义，调用方应结合 `quality` 字段判断。

### 3.5 `JsonReport`

文件：

- `include/JsonReport.h`
- `src/JsonReport.cpp`

职责：把分析结果序列化为 JSON。

报告结构：

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

设计考虑：

- JSON 字段使用 `snake_case`，便于脚本、前端和数据库处理。
- `metadata` 包含工具名称、版本、输入文件和分析耗时。
- `quality` 聚合基础判断结果，调用方可以快速判断模型是否适合后续流程。
- 当前使用 `std::ostringstream` 生成 JSON，保持依赖简单；如果字段变复杂，可以替换为 `nlohmann/json`。

---

## 4. 命令行接口

单文件分析：

```bat
cad_model_analyzer.exe input.step -o output.json
```

兼容旧写法：

```bat
cad_model_analyzer.exe input.step output.json
```

批量分析：

```bat
cad_model_analyzer.exe --batch models -o reports
```

帮助和版本：

```bat
cad_model_analyzer.exe --help
cad_model_analyzer.exe --version
```

批量模式输出：

- `summary.csv`：每个模型一行，包含处理状态、质量状态、复杂度等级、问题数、实体数、面数、边数、自由边、流形边、非流形边、最大边-面邻接数、体积、面积、耗时和报告文件。
- `<model-name>.json`：每个模型的完整 JSON 分析报告。

---

## 5. 构建和运行脚本

项目提供 Windows 脚本，减少手动配置环境的步骤。

### `build-windows.bat`

用途：配置 MSVC x64 环境，调用 CMake + Ninja 构建 Release 版本。

### `run-analyzer.bat`

用途：设置 OCCT 和第三方 DLL 的 PATH，然后运行分析程序。

Windows 下仅编译通过不代表可以直接运行，因为 OpenCASCADE DLL 还依赖 TBB、FreeType、jemalloc、MSVC runtime 等第三方库。运行脚本集中处理这些路径，避免调用者手动设置环境变量。

### `test-screw.bat`

用途：运行最小回归测试，验证：

- 程序可以启动。
- `--help`、`--version` 可用。
- `-o` 参数和旧版参数写法可用。
- 单文件分析输出关键字段。
- 批量模式可以生成 JSON 和 CSV。
- `screw.step` 的关键拓扑、几何、metrics、quality 字段符合预期。

---

## 6. 输出字段说明

### `metadata`

记录分析过程信息：

- `analyzer`：工具名称。
- `version`：工具版本。
- `input_file`：输入文件路径。
- `analysis_time_ms`：分析耗时。

### `topology`

记录拓扑统计和基础拓扑质量指标：

- Vertex / Edge / Wire / Face / Shell / Solid / Compound 数量。
- `isolated_edge`：不邻接 Face 的独立边数量。
- `free_edge`：自由边数量，包含独立边和只邻接 1 个 Face 的边。
- `manifold_edge`：邻接 2 个 Face 的流形边数量。
- `non_manifold_edge`：非流形边数量。
- `max_edge_face_adjacency`：单条边最多邻接 Face 数。
- `euler_characteristic`：欧拉特征数。

### `curves` / `surfaces`

记录常见曲线和曲面类型数量，适合用于模型复杂度和几何构成分析。

### `metrics`

记录包围盒、表面积、体积、质心等工程属性。

### `quality`

记录基础质量判断：

- `closed_solid_candidate`
- `has_free_edges`
- `has_non_manifold_edges`
- `has_positive_volume`

---

## 7. 后续扩展点

### 模型质量检查增强

- 空模型检查。
- 多 Solid 检查。
- Shell-only 模型检查。
- 开放边详情列表。
- 非流形边详情列表。
- 零体积或异常体积检查。
- 面/边数量复杂度等级。

### 更多格式支持

- IGES
- BREP
- STL

### 服务化和可视化

- 封装 HTTP API：上传 STEP/STP，返回 JSON 分析报告。
- 增加简单 Web 页面查看 topology、metrics、quality。
- 对接数据库，形成 CAD 模型资产索引。
- 导出缩略图或轻量级预览数据。

---

## 8. 代码阅读顺序

建议按以下顺序阅读：

1. `src/main.cpp`：命令行参数和主流程。
2. `src/StepReader.cpp`：STEP 读取。
3. `src/TopologyCounter.cpp`：拓扑统计和质量检查。
4. `src/GeometryClassifier.cpp`：曲线和曲面识别。
5. `src/ShapeMetrics.cpp`：包围盒、面积、体积、质心。
6. `src/JsonReport.cpp`：JSON 输出。
7. `test-screw.bat`：回归测试逻辑。

---

## 9. 设计边界

当前项目刻意保持轻量：

- 不实现完整 CAD 建模器。
- 不实现三维渲染引擎。
- 不提供 GUI。
- 不引入数据库。
- 不覆盖所有 CAD 模型质量问题。

这样可以把项目边界集中在 CAD 数据解析、B-Rep 分析、基础模型质量检查和结构化输出上，便于后续作为独立模块集成到更大的系统中。
