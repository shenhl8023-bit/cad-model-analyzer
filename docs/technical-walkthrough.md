# CAD Model Analyzer 技术讲解稿

这份文档用于快速说明项目目标、模块设计、关键技术点和可扩展方向。

---

## 30 秒版本

> CAD Model Analyzer 是一个基于 C++17 和 OpenCASCADE 的 CAD 模型分析工具。它可以读取 STEP 文件，把模型解析成 OpenCASCADE 的 `TopoDS_Shape`，然后遍历 B-Rep 拓扑结构，统计 Vertex、Edge、Face、Solid 等对象，识别常见曲线和曲面类型，计算包围盒、表面积、体积和质心，并检查自由边、非流形边等基础模型质量问题，最后输出 JSON/CSV 报告。

---

## 2 分钟版本

> 项目选择 STEP 作为输入格式，因为 STEP 是工业场景中常见的 CAD 中性文件格式。整体流程是先通过 `STEPControl_Reader` 读取 STEP 文件，得到 OpenCASCADE 的 `TopoDS_Shape`。
>
> 分析逻辑拆成几个模块：`TopologyCounter` 负责拓扑统计和基础质量检查；`GeometryClassifier` 通过 `BRep_Tool::Curve` 和 `BRep_Tool::Surface` 识别 Edge 背后的曲线类型和 Face 背后的曲面类型；`ShapeMetrics` 使用 `BRepBndLib` 和 `BRepGProp` 计算包围盒、表面积、体积和质心；最后 `JsonReport` 把结果输出为 JSON。
>
> 工程化方面，项目使用 CMake + Ninja + MSVC 构建，并提供 `build-windows.bat`、`run-analyzer.bat` 和 `test-screw.bat`。运行脚本统一处理 OpenCASCADE 在 Windows 下的 DLL 依赖，回归测试使用 OCCT 自带的 `screw.step` 验证核心输出字段。

---

## 5 分钟深入讲解版本

### 1. 项目目标

目标是把 CAD STEP 文件转换成可程序消费的结构化数据，而不是只做文件读取或图形显示。输出 JSON/CSV 后，可以继续接入模型质检、数据库、报价系统或 Web 服务。

### 2. 数据流

```text
STEP 文件
   ↓ STEPControl_Reader
TopoDS_Shape
   ↓
拓扑遍历 / 几何分类 / 属性计算
   ↓
JSON 报告 / CSV 汇总
```

STEP 文件读入后，OpenCASCADE 给到的是一个 `TopoDS_Shape`。它可能包含 Compound、Solid、Shell、Face、Wire、Edge、Vertex 等不同层级，后续分析都围绕这个 Shape 展开。

### 3. 拓扑统计

对应文件：

- `src/TopologyCounter.cpp`

拓扑统计使用 `TopExp_Explorer`，它可以从一个 `TopoDS_Shape` 中遍历指定类型，例如 `TopAbs_FACE` 或 `TopAbs_EDGE`。项目把统计逻辑封装成 `countShapeType`，分别统计 Vertex、Edge、Wire、Face、Shell、Solid 和 Compound。

自由边和非流形边检查使用 `TopExp::MapShapesAndAncestors` 建立 Edge 到 Face 的邻接关系：

- 被少于 2 个 Face 使用的边记为自由边。
- 被超过 2 个 Face 使用的边记为非流形边。

### 4. 几何分类

对应文件：

- `src/GeometryClassifier.cpp`

Edge 和 Face 是拓扑对象，本身不直接说明它是直线、圆、平面还是圆柱面。项目通过：

- `BRep_Tool::Curve(edge, first, last)` 获取 Edge 背后的 `Geom_Curve`。
- `BRep_Tool::Surface(face)` 获取 Face 背后的 `Geom_Surface`。
- `IsKind(STANDARD_TYPE(...))` 判断具体曲线/曲面类型。

### 5. 尺寸和质量属性

对应文件：

- `src/ShapeMetrics.cpp`

包围盒使用 `BRepBndLib::Add` 计算，得到最小最大坐标后计算 dx、dy、dz、中心点和空间对角线。

表面积使用 `BRepGProp::SurfaceProperties`，体积和质心使用 `BRepGProp::VolumeProperties`。这些属性可以用于模型检查、报价、重量估算和加工复杂度评估。

### 6. 输出设计

对应文件：

- `src/JsonReport.cpp`

项目输出 JSON，而不是只打印文本，目的是方便后续被 Web 服务、数据库或前端页面消费。批量模式还会生成 `summary.csv`，适合对目录中的多个模型进行快速汇总。

### 7. 工程化部分

对应文件：

- `CMakeLists.txt`
- `build-windows.bat`
- `run-analyzer.bat`
- `test-screw.bat`

C++ 项目使用 CMake 管理源码和 OpenCASCADE 链接库。Windows 下 OCCT 运行时依赖较多，脚本中统一配置 OCCT 和第三方库 PATH。`test-screw.bat` 是最小回归测试，用于验证程序能启动、命令行参数可用、分析结果关键字段正确。

---

## 常见技术问题

### Q1：为什么选 OpenCASCADE？

OpenCASCADE 是开源 CAD 几何内核，支持 STEP/IGES/BREP 等格式，提供拓扑遍历、几何曲线曲面、布尔运算和质量属性计算等能力，适合作为 CAD 数据处理工具的底层库。

### Q2：STEP 文件读进来以后是什么结构？

读进来以后核心对象是 `TopoDS_Shape`。它是一个通用拓扑对象，可以表示 Compound、Solid、Shell、Face、Wire、Edge、Vertex。具体分析时用 `TopExp_Explorer` 按类型遍历。

### Q3：拓扑和几何有什么区别？

拓扑描述连接关系，比如模型有哪些 Face、Edge、Vertex，它们如何组成实体；几何描述具体形状，比如一条 Edge 底层是直线还是圆，一张 Face 底层是平面还是圆柱面。OpenCASCADE 里需要通过 `BRep_Tool` 从拓扑对象取到底层几何对象。

### Q4：怎么识别曲线和曲面类型？

对 Edge 调用 `BRep_Tool::Curve` 获取 `Geom_Curve`，然后用 `IsKind(STANDARD_TYPE(Geom_Line))` 等判断类型。Face 类似，调用 `BRep_Tool::Surface` 获取 `Geom_Surface`，判断 Plane、Cylinder、Sphere、Cone、Torus、BSplineSurface 等。

### Q5：面积、体积和质心怎么计算？

OpenCASCADE 提供了 `BRepGProp`。表面积使用 `BRepGProp::SurfaceProperties`，体积和质心使用 `BRepGProp::VolumeProperties`。得到的 `GProp_GProps` 里 `Mass()` 对 surface properties 表示面积，对 volume properties 表示体积，`CentreOfMass()` 可以拿到质心。

### Q6：项目里有哪些工程难点？

主要是 Windows 下 OCCT 运行依赖。程序能编译通过不代表能运行，因为 exe 依赖 OCCT DLL，OCCT DLL 又依赖 TBB、FreeType、jemalloc、MSVC runtime 等第三方库。项目通过运行脚本统一配置 PATH，并在构建脚本中复制核心 OCCT DLL，减少环境差异导致的运行失败。

### Q7：项目还可以怎么扩展？

可以继续增强模型质量检查，例如空模型、多实体、Shell-only、开放边详情和非流形边详情；也可以支持 IGES/BREP/STL 输入，或封装成 Web API，让上层系统上传 STEP 文件后返回模型尺寸、体积、面积、复杂度和质量状态。
