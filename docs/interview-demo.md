# CAD Model Analyzer 面试讲解稿

这份文档用于面试时快速介绍项目。建议提前熟悉，但不要逐字背诵。面试时重点讲“为什么做、怎么做、难点是什么、能扩展到什么业务场景”。

---

## 30 秒版本

> 我做了一个基于 C++ 和 OpenCASCADE 的 CAD 模型分析工具。它可以读取 STEP 文件，把模型解析成 OpenCASCADE 的 `TopoDS_Shape`，然后遍历 B-Rep 拓扑结构，统计 Vertex、Edge、Face、Solid 等数量，同时识别常见曲线和曲面类型，计算包围盒、表面积、体积和质心，并检查自由边、非流形边等基础模型质量问题，最后输出 JSON 报告。这个项目主要是为了展示我对 CAD 几何内核、B-Rep 拓扑、C++ 工程化和模型数据分析流程的理解。

---

## 2 分钟版本

> 这个项目的背景是我想补足 CAD/CAE 方向的工程实践，所以选择 OpenCASCADE 做了一个 STEP 模型分析工具。
>
> 整体流程是：先通过 `STEPControl_Reader` 读取 STEP 文件，转换成 OpenCASCADE 的 `TopoDS_Shape`。然后我把分析逻辑拆成几个模块：`TopologyCounter` 负责拓扑统计和基础质量检查，`GeometryClassifier` 负责识别 Edge 的曲线类型和 Face 的曲面类型，`ShapeMetrics` 负责计算包围盒、面积、体积、质心，最后 `JsonReport` 把结果输出成 JSON。
>
> 这个项目不是只做文件读取，而是围绕 B-Rep 模型做结构化分析。比如 Edge 本身只是拓扑边，要通过 `BRep_Tool::Curve` 拿到底层几何曲线；Face 也要通过 `BRep_Tool::Surface` 拿到底层曲面。模型质量检查则通过 `TopExp::MapShapesAndAncestors` 建立 Edge 到 Face 的邻接关系，用来判断自由边和非流形边。质量属性用 `BRepGProp::SurfaceProperties` 和 `BRepGProp::VolumeProperties` 来计算。
>
> 工程化方面，我用 CMake + Ninja + MSVC 构建，并写了 `build-windows.bat`、`run-analyzer.bat` 和 `test-screw.bat`。因为 OpenCASCADE 在 Windows 下有比较多 DLL 依赖，所以我也处理了运行时 PATH 和依赖复制问题。最后用 OCCT 自带的 `screw.step` 做了一个最小回归测试，确保拓扑统计、几何分类和 metrics 字段稳定输出。

---

## 5 分钟深入讲解版本

### 1. 项目目标

> 我希望做一个能体现 CAD 底层能力的小项目，不是简单调用软件界面，而是直接处理 CAD 数据。STEP 是工业里常见的中性 CAD 文件格式，所以我选择读取 STEP，并把复杂模型转换成结构化 JSON。

### 2. 数据流

```text
STEP 文件
   ↓ STEPControl_Reader
TopoDS_Shape
   ↓
拓扑遍历 / 几何分类 / 属性计算
   ↓
JSON 报告
```

可以这样解释：

> STEP 文件读进来之后，OpenCASCADE 给到的是一个 `TopoDS_Shape`。它可能包含 Compound、Solid、Shell、Face、Wire、Edge、Vertex 等不同层级。后续所有分析都是围绕这个 Shape 展开。

### 3. 拓扑统计

对应文件：

- `src/TopologyCounter.cpp`

讲解点：

> 拓扑统计使用 `TopExp_Explorer`。它可以从一个 `TopoDS_Shape` 中遍历指定类型，比如 `TopAbs_FACE` 或 `TopAbs_EDGE`。我把统计逻辑封装成 `countShapeType`，分别统计 Vertex、Edge、Wire、Face、Shell、Solid 和 Compound。同时我还计算了 `V - E + F` 欧拉特征数，并用 `TopExp::MapShapesAndAncestors` 建立 Edge 到 Face 的邻接关系，用来统计 free edge 和 non-manifold edge。

可以提到：

> 这里体现了 B-Rep 的拓扑层级概念。CAD 模型不是一个三角网格，而是由拓扑对象和几何对象共同构成。自由边和非流形边检查也能体现模型质量检查思路，面试官会更容易看出这不是只做了文件读取。

### 4. 几何分类

对应文件：

- `src/GeometryClassifier.cpp`

讲解点：

> Edge 和 Face 是拓扑对象，本身不直接说明它是直线还是圆，也不直接说明它是平面还是圆柱面。要通过 `BRep_Tool::Curve(edge, first, last)` 获取 Edge 背后的 `Geom_Curve`，再用 `IsKind(STANDARD_TYPE(...))` 判断是 `Geom_Line`、`Geom_Circle`、`Geom_BSplineCurve` 等。
>
> Face 也是类似，通过 `BRep_Tool::Surface(face)` 获取 `Geom_Surface`，再判断 `Geom_Plane`、`Geom_CylindricalSurface`、`Geom_ToroidalSurface` 等。

可以补一句：

> 这块是项目里比较能体现 CAD 内核理解的部分，因为它区分了拓扑和几何两个层面。

### 5. 尺寸和质量属性

对应文件：

- `src/ShapeMetrics.cpp`

讲解点：

> 包围盒用 `BRepBndLib::Add` 计算，得到最小最大坐标后，可以算出 dx、dy、dz、中心点和空间对角线。
>
> 表面积通过 `BRepGProp::SurfaceProperties` 计算，体积和质心通过 `BRepGProp::VolumeProperties` 计算。这些属性在 CAD 模型检查、报价、重量估算、加工复杂度评估里都比较有价值。

### 6. 输出设计

对应文件：

- `src/JsonReport.cpp`

讲解点：

> 我把结果输出成 JSON，而不是只打印文本，是因为 JSON 更适合后续被 Web 服务、数据库或前端页面消费。这个工具后续可以作为 CAD 分析服务的核心模块。

### 7. 工程化部分

对应文件：

- `CMakeLists.txt`
- `build-windows.bat`
- `run-analyzer.bat`
- `test-screw.bat`

讲解点：

> C++ 项目里我用 CMake 管理源码和 OpenCASCADE 链接库。Windows 下 OCCT 运行时依赖很多 DLL，所以脚本里配置了 OCCT 和第三方库的 PATH。`test-screw.bat` 是一个最小回归测试，确保程序能启动、命令行参数可用、分析结果关键字段正确。

---

## 面试官可能问的问题和回答

### Q1：为什么选 OpenCASCADE？

答：

> OpenCASCADE 是开源 CAD 几何内核，支持 STEP/IGES/BREP 等格式，提供拓扑遍历、几何曲线曲面、布尔运算、质量属性计算等能力。对学习 CAD/CAE 底层数据结构比较合适，也比直接调商业 CAD 软件 API 更能体现底层理解。

### Q2：STEP 文件读进来以后是什么结构？

答：

> 读进来以后核心对象是 `TopoDS_Shape`。它是一个通用拓扑对象，可以表示 Compound、Solid、Shell、Face、Wire、Edge、Vertex。具体分析时用 `TopExp_Explorer` 按类型遍历。

### Q3：拓扑和几何有什么区别？

答：

> 拓扑描述连接关系，比如一个模型有哪些 Face、Edge、Vertex，它们如何组成实体；几何描述具体形状，比如一条 Edge 底层是直线还是圆，一张 Face 底层是平面还是圆柱面。OpenCASCADE 里需要通过 `BRep_Tool` 从拓扑对象取到底层几何对象。

### Q4：你怎么识别曲线和曲面类型？

答：

> 对 Edge 调用 `BRep_Tool::Curve` 获取 `Geom_Curve`，然后用 `IsKind(STANDARD_TYPE(Geom_Line))` 等判断类型。Face 类似，调用 `BRep_Tool::Surface` 获取 `Geom_Surface`，判断 Plane、Cylinder、Sphere、Cone、Torus、BSplineSurface 等。

### Q5：面积、体积和质心怎么计算？

答：

> OpenCASCADE 提供了 `BRepGProp`。表面积使用 `BRepGProp::SurfaceProperties`，体积和质心使用 `BRepGProp::VolumeProperties`。得到的 `GProp_GProps` 里 `Mass()` 对 surface properties 表示面积，对 volume properties 表示体积，`CentreOfMass()` 可以拿到质心。

### Q6：项目里遇到过什么问题？

答：

> 主要是 Windows 下 OCCT 运行依赖问题。程序编译成功不代表能运行，因为 exe 依赖 OCCT DLL，OCCT DLL 又依赖第三方 DLL，比如 TBB、FreeType、jemalloc、MSVC runtime 等。我最后通过分析依赖链，在运行脚本里统一配置 PATH，并在构建脚本中复制核心 OCCT DLL，解决了运行时找不到 DLL 的问题。

### Q7：这个项目如果继续做，你会怎么扩展？

答：

> 我会继续增强模型质量检查，比如空模型、多实体、Shell-only、开放边详情和非流形边详情。批量分析和 CSV 汇总现在已经有基础版本，下一步可以封装成 Web 服务，让用户上传 STEP 文件后返回模型尺寸、体积、面积、复杂度和质量状态。

### Q8：这个项目和岗位有什么关系？

答：

> 如果岗位涉及 CAD/CAE、三维几何、工业软件、C++ 后端或图形几何处理，这个项目能体现我对 CAD 文件解析、B-Rep 数据结构、OpenCASCADE API、C++ 工程化和模型分析流程的实践能力。虽然项目规模不大，但它覆盖了 CAD 底层处理链路中的关键环节。

---

## 建议现场演示流程

### 第一步：展示 README

重点让面试官看到：

- 项目目标
- 技术栈
- 架构图
- 功能清单

### 第二步：运行测试

```bat
D:\CodeProj\cad-model-analyzer\test-screw.bat
```

预期输出：

```text
OK: screw.step report matches expected counts, metrics, and CLI options
```

### 第三步：运行单文件分析

```bat
D:\CodeProj\cad-model-analyzer\run-analyzer.bat D:\CodeProj\CoreEngine\occt-combined-release-no-pch\opencascade-8.0.0-vc14-64\data\step\screw.step D:\CodeProj\cad-model-analyzer\output\sample-output.json
```

### 第四步：打开 JSON 输出

重点解释：

- `topology`：B-Rep 拓扑统计
- `curves`：Edge 底层曲线类型
- `surfaces`：Face 底层曲面类型
- `metrics`：工程尺寸和质量属性

### 第五步：打开核心源码

建议按这个顺序讲：

1. `src/main.cpp`：程序入口和主流程
2. `src/StepReader.cpp`：STEP 读取
3. `src/TopologyCounter.cpp`：拓扑遍历
4. `src/GeometryClassifier.cpp`：曲线/曲面识别
5. `src/ShapeMetrics.cpp`：包围盒、面积、体积、质心
6. `src/JsonReport.cpp`：JSON 输出

---

## 简历项目描述参考

可以写成：

> **CAD Model Analyzer：基于 OpenCASCADE 的 STEP 模型几何分析工具**
>
> 使用 C++17、OpenCASCADE、CMake 实现 STEP 模型分析工具，支持读取 `.step/.stp` 文件并遍历 B-Rep 拓扑结构，统计 Vertex、Edge、Face、Solid 等对象数量；基于 `BRep_Tool` 识别 Edge 曲线类型和 Face 曲面类型；使用 `BRepBndLib`、`BRepGProp` 计算包围盒、表面积、体积和质心；输出结构化 JSON 报告，并编写 Windows 构建/运行脚本和最小回归测试，解决 OCCT 多层 DLL 运行依赖问题。

---

## 需要避免的说法

不要说：

- “我只是让 AI 帮我写了个小工具。”
- “这个项目没什么，就是读文件。”
- “OpenCASCADE 我也不太懂。”
- “代码具体我忘了。”

建议说：

- “这个项目是我为了理解 CAD 几何内核做的实践。”
- “我重点学习了 STEP 读取、B-Rep 拓扑遍历、几何分类和质量属性计算。”
- “项目规模不大，但覆盖了 CAD 文件到结构化分析结果的完整链路。”
- “现在支持单模型 JSON 输出和目录批量分析 CSV 汇总。”
- “如果继续做，我会扩展更细的模型质量检查、更多 CAD 格式和 Web/API 集成。”
