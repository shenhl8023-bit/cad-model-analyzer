# CAD Model Analyzer 能力覆盖说明

本文档说明 CAD Model Analyzer 当前覆盖的工程能力、适用场景和后续扩展方向，便于代码阅读者快速判断项目边界。

---

## 适用场景

这个项目适合用于以下方向的技术验证或二次开发：

1. CAD/CAE/CAM 数据处理工具。
2. 三维几何与 B-Rep 拓扑分析。
3. 工业软件 C++ 工程模块。
4. 几何内核 / CAD 数据转换 / STEP-STP 解析。
5. 制造业数字化、模型质检、报价系统、PLM/PDM 数据处理。

项目定位不是完整 CAD 建模器，而是一个围绕 **STEP 文件解析、B-Rep 拓扑分析、几何分类、模型质量检查、结构化输出** 的 C++ 工程项目。

---

## 技术能力覆盖

### 1. C++17 工程实现

项目使用 C++17 编写命令行 CAD 分析工具，包含：

- 模块化源码组织：STEP 读取、拓扑统计、几何分类、属性计算、JSON 输出分离。
- 使用结构体组织分析结果，例如 `TopologyStats`、`CurveStats`、`SurfaceStats`、`ShapeMetrics`。
- 使用 `std::string`、文件输入输出、异常处理、命令行参数解析和计时统计。
- 支持单文件分析和批量目录分析。
- 通过脚本化 smoke test 验证核心 CLI 行为和输出字段。

### 2. CMake / MSVC / Windows C++ 工程

项目使用 CMake + Ninja + Visual Studio 2022 MSVC x64 构建，并集成 OpenCASCADE 8.0：

- `build-windows.bat`：构建 Windows x64 Release 可执行文件。
- `run-analyzer.bat`：统一配置 OCCT 和第三方 DLL 路径后运行。
- `test-screw.bat`：运行最小回归测试。
- 处理 OpenCASCADE 多层运行时依赖，包括 OCCT 自身 DLL 和第三方 DLL。

### 3. OpenCASCADE / CAD 几何内核

项目使用 OpenCASCADE 完成 CAD 数据读取和分析：

- `STEPControl_Reader` 读取 STEP/STP 文件。
- `TopoDS_Shape` 作为 B-Rep 模型入口。
- `TopExp_Explorer` 遍历 Vertex、Edge、Wire、Face、Shell、Solid、Compound。
- `BRep_Tool::Curve` 获取 Edge 底层 `Geom_Curve`。
- `BRep_Tool::Surface` 获取 Face 底层 `Geom_Surface`。
- `BRepBndLib` 计算包围盒。
- `BRepGProp::SurfaceProperties` 和 `BRepGProp::VolumeProperties` 计算面积、体积和质心。

### 4. B-Rep 拓扑结构和模型质量检查

项目统计 B-Rep 基础拓扑对象并输出基础质量指标：

- Vertex / Edge / Wire / Face / Shell / Solid / Compound 数量。
- 常见曲线类型：Line、Circle、Ellipse、BSplineCurve。
- 常见曲面类型：Plane、Cylinder、Sphere、Cone、Torus、BSplineSurface。
- 欧拉特征数 `V - E + F`。
- 基于 Edge-Face 邻接关系判断自由边和非流形边。
- 根据实体数、自由边、非流形边、体积等字段判断闭合实体候选模型。
- 输出综合质量状态：`ok`、`open_shell`、`non_manifold`、`multi_solid`、`shell_only`、`empty`、`zero_or_invalid_volume`。
- 输出 `issue_count` 和 `complexity_level`，便于批量处理时快速筛选异常模型和高复杂度模型。

### 5. STEP 文件处理和结构化输出

项目支持 `.step` / `.stp` 文件读取，并将分析结果输出为 JSON/CSV：

- 单文件模式输出完整 JSON 报告。
- 批量模式扫描目录下的 STEP/STP 文件。
- 每个模型输出单独 JSON。
- 生成 `summary.csv` 汇总质量状态、复杂度等级、问题数、实体数、面数、边数、自由边、流形边、非流形边、最大边-面邻接数、体积、面积、耗时和报告文件。
- 输出字段可被 Web 服务、数据库、报价系统或模型质检流程消费。

### 6. 工程工具化设计

项目不是只打印控制台文本，而是按可集成工具设计：

- 支持 `--help`、`--version`、`-o`、`--batch` 等 CLI 参数。
- 兼容旧版参数写法。
- 批量分析中单个模型失败时继续处理并记录错误。
- 提供架构说明、Roadmap、示例 JSON 和 CSV 输出。
- 提供脚本化最小回归测试。

---

## 后续扩展方向

- 增强模型质量检查：开放边详情、非流形边详情、问题样本定位和更细粒度复杂度评分。
- 增加 IGES / BREP / STL 等更多格式支持。
- 封装 HTTP API：上传 STEP 文件后返回模型尺寸、体积、面积、复杂度和质量状态。
- 增加简单 Web 页面查看 topology / metrics / quality。
- 对接数据库，形成 CAD 模型资产索引。
