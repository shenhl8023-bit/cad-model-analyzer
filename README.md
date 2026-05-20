# CAD Model Analyzer

基于 **C++17 + OpenCASCADE** 的 STEP 模型几何分析工具。项目读取 `.step/.stp` CAD 文件，解析为 OpenCASCADE 的 `TopoDS_Shape`，统计模型拓扑结构、曲线/曲面类型，并计算包围盒、表面积、体积、质心等工程指标，最后输出结构化 JSON。

这个项目定位为 **CAD/CAE/几何内核方向面试展示项目**：重点展示 C++ 工程化、OpenCASCADE 使用、B-Rep 拓扑遍历、几何类型识别、质量属性计算和 Windows 原生程序构建排障能力。

---

## 项目亮点

- **CAD 文件解析**：使用 OpenCASCADE 读取 STEP 格式模型。
- **B-Rep 拓扑分析**：统计 Vertex、Edge、Wire、Face、Shell、Solid、Compound。
- **几何类型识别**：识别直线、圆、椭圆、B 样条曲线，以及平面、圆柱面、球面、圆锥面、圆环面、B 样条曲面。
- **工程指标计算**：输出 bounding box、包围盒中心、对角线长度、表面积、体积、质心。
- **结构化输出**：将 CAD 模型分析结果转换为 JSON，便于后续接入 Web、数据库、报价系统或模型质量检查流程。
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

### 2. 拓扑统计

统计 B-Rep 模型中的基础拓扑对象：

- Vertex
- Edge
- Wire
- Face
- Shell
- Solid
- Compound

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
StepReader
   ↓
TopoDS_Shape
   ↓
┌────────────────────┬──────────────────────┬────────────────┐
│ TopologyCounter    │ GeometryClassifier   │ ShapeMetrics   │
│ 拓扑统计            │ 曲线/曲面分类          │ 尺寸/质量属性    │
└────────────────────┴──────────────────────┴────────────────┘
   ↓
JsonReport
   ↓
JSON 报告
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
  output/
    sample-output.json
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
- metrics 字段存在并且关键数值大于 0

成功输出示例：

```text
OK: screw.step report matches expected counts, metrics, and CLI options
```

---

## 输出示例

完整示例见：[`output/sample-output.json`](output/sample-output.json)

节选：

```json
{
  "topology": {
    "vertex": 88,
    "edge": 44,
    "wire": 10,
    "face": 10,
    "shell": 1,
    "solid": 1,
    "compound": 0
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
  "metrics": {
    "bounding_box": {
      "dx": 20.0006,
      "dy": 20.0006,
      "dz": 42.3005,
      "center": {
        "x": -17.8981,
        "y": -0.826297,
        "z": -13.4137
      },
      "diagonal": 50.8859
    },
    "surface_area": 1929.33,
    "volume": 3788.27,
    "center_of_mass": {
      "x": -17.8981,
      "y": -0.826298,
      "z": -11.1583
    }
  }
}
```

---

## 面试时可以重点讲什么

### 1. 为什么做这个项目

CAD/CAE 软件不是普通文本或图片处理，核心数据结构是 B-Rep 几何拓扑。这个项目用 OpenCASCADE 读取 STEP 文件，把复杂 CAD 模型转换成可分析、可存储、可展示的结构化数据。

### 2. 技术难点

- 理解 `TopoDS_Shape` 只是一个通用形状入口，需要通过 `TopExp_Explorer` 遍历不同拓扑类型。
- Edge 和 Face 本身不是几何，需要通过 `BRep_Tool::Curve` / `BRep_Tool::Surface` 取到底层几何对象。
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

- 模型质量检查：开放边、非流形边、空体、多个 Solid 等。
- 批量分析目录并输出 CSV。
- 支持 IGES / BREP / STL。
- 增加 Web 或桌面 UI。
- 对接数据库，保存模型分析结果。
- 输出可视化预览图。

---

## 面试讲解资料

- 架构说明：[`docs/architecture.md`](docs/architecture.md)
- 面试讲解稿：[`docs/interview-demo.md`](docs/interview-demo.md)
- 示例输出：[`output/sample-output.json`](output/sample-output.json)
