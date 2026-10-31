# 架构说明：经典分层 MVC

本文档定义 life-game 的架构契约。所有代码、测试与评审都以本文为准。
目标不是"看起来分层"，而是让依赖方向可以被机器检查、被编译器约束、被新人一眼看懂。

## 1. 设计目标

1. **经典 MVC 的观察关系**：Model 不知道 View/Controller；View 直接观察 Model
   并只读访问它；Controller 解释用户输入并调用 Model 的命令。
2. **单向依赖**：`domain ← model ← view/controller ← app`，任何反向依赖都视为缺陷。
3. **领域层零框架依赖**：`domain` 不包含任何 Qt 头文件，可用普通 C++ 编译器独立测试。
4. **可测试性**：Model 不碰文件、不弹对话框、不拼接 UI 文案；Controller 对状态栏与
   跳转进度使用窄接口（`MessageSink` / `JumpProgressView`），其余协作者保持具体类以避免
   为小项目过度抽象；核心路径均有 QtTest/CTest。
5. **可执行的质量门禁**：CMake 目标、`tools/check_layering.py`、CTest 与 CI 共同
   防止架构腐化。

## 2. 分层与依赖规则

| 层 | 目录 | 可以依赖 | 禁止依赖 | 职责 |
|---|---|---|---|---|
| Domain | `src/domain` | 自身、C++ 标准库 | Qt、Model、View、Controller、Service、App | 实体、值对象、规则、棋盘、图案、RLE 编解码、快照历史 |
| Model | `src/model` | Domain、QtCore/QtConcurrent、自身 | View、Controller、Service、App、文件/对话框/UI 文案 | 可观察的应用状态、命令、异步演化 |
| Service | `src/service` | Domain、Model 的值对象、QtCore/QtGui、自身 | View、Controller、App | 文件读写、QSettings 持久化、图片编码等基础设施 |
| View | `src/view` | Domain、Model（只读）、QtWidgets、自身 | Controller、Service、App | 绘制、控件、观察 Model、抛出用户输入信号 |
| Controller | `src/controller` | Domain、Model、View、Service、自身 | App | 解释输入、调用 Model 命令、驱动定时器/对话框 |
| App | `src/app` | 全部 | — | `main()`、组合根、菜单/工具栏装配 |

**依赖方向：**

```
        ┌──────────────────────────── app ────────────────────────────┐
        │  main.cpp · MainWindow（组合根）· 菜单/工具栏/状态栏          │
        └───────┬───────────────┬───────────────┬───────────────┬──────┘
                │               │               │               │
             controller ───────┼───────► view  │               │
                │               │               │               │
                └───────────────┴──────► model ◄───────────────┘
                                            │
                                            ▼
                                         domain
        service ──► domain/model 的值对象（不反向依赖 view/controller）
```

更严格地说：

- `domain` 不知道任何上层。
- `model` 只认识 `domain` 和 QtCore/QtConcurrent。
- `view` 可以包含 `model/life_model.h`，但只持有 `const LifeModel*`，只能调用 const 查询、
  只能连接 Model 的信号；不得调用任何修改状态的方法。
- `controller` 是除 App 组合根之外，唯一可以同时持有 `LifeModel*`（可写）和 View
  指针的层；`MainWindow` 作为组合根持有它们用于注入。
- `service` 不包含 UI 类型；文件/设置错误以 `FileError` / `SettingsError` 等类型化
  结果返回，由 Controller 转成用户文案。
- `app` 只做装配，不实现业务规则；菜单动作直接转发给 Controller 或纯 View 操作。
  `MainWindow` 可以构造 `QSettings` 并注入 `SettingsStore`，但不读写具体配置键。

## 3. 经典 MVC 的职责边界

### Model

- 持有棋盘、规则、边界、代数、运行状态、历史快照。
- 提供 **只读查询** 与 **命令**。
- 通过信号广播变化：`boardChanged`、`statusChanged`、`runningChanged`、
  `configurationChanged`、`simulationStalled`、`jumpStarted/Progress/Finished/Cancelled`。
- 不包含：`QFile`、`QFileDialog`、`QSettings`、中文提示、View/Controller 头文件、View 指针。
- 单代异步计算封装在 `SimulationEngine` 中；代数跳转的重放任务在 `LifeModel` 内使用
  `QtConcurrent::run`，Model 只负责状态与一致性。

### View

- 直接连接 Model 信号并读取 Model 的 const 查询（`board()`、`stats()`、`isRunning()` 等）。
- 只负责"把 Model 画出来"和"把用户操作变成信号"。
- View 自身可以拥有纯视图状态：缩放、平移、悬停、网格显隐、图案预览。
- 不得调用 Model 的写方法，不得包含 Controller/Service/App 头文件。
- 不复制整块棋盘：`BoardView` 直接读取 `const domain::Grid&`，避免每帧 160 KB 拷贝。

### Controller

- 解释输入（鼠标、菜单、控件信号），调用 Model 命令。
- 驱动 `QTimer`（模拟时钟）与对话框（文件、设置、跳转进度）。
- 可以把瞬时 UI 状态喂给 View（例如图案预览、状态栏消息），但**不得搬运整块棋盘数据**。
- 每个 Controller 只负责一类输入，避免"上帝控制器"。
- 负责把 `CommandResult`/`FileError`/`SettingsError` 映射成一条可操作的用户提示，
  并在失败/状态迁移处记录日志。
- `FileController` 使用具体的 `RleFileService`/`ImageFileService`；它们由 App 注入，
  使文件操作可替换、可测试。

### App

- `MainWindow` 是组合根：创建 Model、View、Controller、Service，注入依赖，连接菜单。
- 不包含演化规则、不直接读写 QSettings、不拼接业务状态。
- 菜单动作要么转发给 Controller，要么调用 View 的纯视图操作（缩放、全屏、帮助）。

## 4. 目录结构

```
src/
  domain/                 纯 C++，无 Qt
    board_limits.h        棋盘尺寸/密度等共享常量
    cell.h                Cell 类型与死活值
    boundary.h            边界枚举
    grid.h/.cpp           固定容量棋盘、串行/按行演化
    rule.h/.cpp           B/S 规则、预设、解析
    pattern.h/.cpp        图案实体
    pattern_library.h/.cpp 内置图案（仅 ID + 形状，不含文案）
    rle.h/.cpp            RLE 编解码（带边界检查与错误码）
    history.h/.cpp        快照历史（纯 C++，可独立测试）
  model/                  QtCore + QtConcurrent
    life_stats.h          代数/人口/密度值对象
    stall_reason.h        自动暂停原因（不含文案）
    jump_info.h           跳转参数值对象
    app_settings.h        应用设置值对象与范围
    command_result.h      命令错误码
    model_log.h/.cpp      lifegame.model category
    simulation_engine.h/.cpp  异步一代演化管线
    life_model.h/.cpp     可观察 Model
  service/
    settings_store.h/.cpp QSettings 持久化
    settings_error.h      设置错误码
    session_state.h       会话快照值对象
    file_error.h          文件错误码
    service_log.h/.cpp    lifegame.io category
    rle_file_service.h/.cpp 文件读写 + Domain RLE 解码
    image_file_service.h/.cpp PNG 保存
  view/
    interfaces.h          MessageSink / JumpProgressView / ConfirmSink 接口
    message_box_confirm_sink.h/.cpp QMessageBox 确认实现
    dialogs.h/.cpp        中文警告/信息对话框
    theme.h/.cpp          Fusion 深色外观与图标
    rule_catalog.h/.cpp   规则 ID → 中文显示名
    pattern_catalog.h/.cpp 图案 ID → 名称/分类/说明
    board_view.h/.cpp     自绘棋盘（观察 Model）
    control_panel.h/.cpp  侧边面板（观察 Model）
    settings_dialog.h/.cpp 设置对话框（纯视图）
    jump_progress_dialog.h/.cpp 跳转进度对话框
    status_bar_view.h/.cpp 状态栏消息适配器
  controller/
    controller_log.h/.cpp      lifegame.controller category
    error_text.h/.cpp          错误码 → 中文提示
    board_controller.h/.cpp    鼠标绘制、图案放置、清空/随机/示例
    simulation_controller.h/.cpp 播放/暂停/单步/速度/定时器
    jump_controller.h/.cpp     回退/快进与进度视图
    file_controller.h/.cpp     打开/保存/导出
    configuration_controller.h/.cpp 规则/边界/网格显隐
    session_controller.h/.cpp  启动恢复、设置应用、退出保存
    line_raster.h/.cpp         纯 C++ Bresenham（可独立测试）
  app/
    main_window.h/.cpp   组合根 + 菜单/工具栏
    logging.h/.cpp       唯一 qInstallMessageHandler、轮转与过滤
    main.cpp             入口 + QCommandLineParser
tests/
  domain_tests.cpp       纯 C++，无 Qt
  model_tests.cpp        QtTest
  view_tests.cpp         QtTest（offscreen）
  controller_tests.cpp   QtTest（offscreen）
  ui_tests.cpp           QtTest（offscreen/Xvfb）
  logging_tests.cpp      文件 sink / category / 轮转
tools/
  check_layering.py      静态检查 include / 纯净度 / CMake 清单与链接
  layering_selftest.py   架构门禁的负向 fixture
```

## 5. 典型数据流

### 用户绘制

```
BoardView::cellPressed(row, col)
  → BoardController::onCellPressed
  → LifeModel::setCell(row, col, !alive)     // 命令（画笔路径返回 bool）
  → LifeModel 更新棋盘、人口、历史
  → LifeModel::boardChanged / statusChanged  // 广播
  → BoardView（观察者）update()
  → ControlPanel（观察者）刷新统计
```

注意：Controller **不**把棋盘数据复制给 View；View 直接读取 Model。

### 模拟定时器

```
QTimer::timeout
  → SimulationController::onTick
  → LifeModel::requestStep()
  → SimulationEngine 在 Qt 线程池按行并行计算
  → QFutureWatcher::finished
  → LifeModel::onStepReady() → LifeModel::applyStep()
  → boardChanged / statusChanged
  → View/ControlPanel 自行更新
```

### 打开 RLE

```
FileController::openRle()
  → QFileDialog（输入）
  → RleFileService::read()        // 文件 I/O
  → domain::decodeRle()           // 纯 C++ 解析
  → LifeModel::loadDocument()     // 应用状态
  → boardChanged / configurationChanged / statusChanged
  → View/ControlPanel 自行更新
```

## 6. 关键约定

### 6.1 错误与文案

- 应用代码以 `-fno-exceptions` 编译；可恢复错误不使用异常。
- Domain 返回 `RleError` / `RuleParseError` 等错误码/英文技术信息，不返回中文 UI 文案；
  `Rule::parse` 返回 `RuleParseResult{error, rule}` 值对象。
- Model 命令返回 `CommandResult`；`simulationStalled(StallReason)` 只发枚举。
- Service 返回 `FileError` / `SettingsError`，系统详情只作为可选的补充字符串。
- 中文文案只出现在 `view/`、`controller/` 与 App 外壳（菜单/标题/帮助），通过 `tr()`
  提供翻译入口；Model/Domain 不出现中文。
- `CommandResult` 的 `Clipped`/`Adjusted` 表示"已生效但有调整"；`Busy`/`NoHistory`/
  `InvalidRule` 等区分用户可采取的不同动作，Controller 不再用一条模糊文案覆盖所有原因。

### 6.2 日志

- 五个 category：`lifegame.app`、`lifegame.model`、`lifegame.io`、
  `lifegame.controller`、`lifegame.view`；定义分散在各层，`src/app/logging` 是唯一
  安装 `qInstallMessageHandler` 的地方。
- 默认文件 `QStandardPaths::AppLocalDataLocation/logs/life-game.log`，
  2 MiB × 3 轮转；`--verbose` 打开 `lifegame.*.debug`，`--log-file` 覆盖路径。
- 只记录低频状态迁移、文件/设置失败、模拟/跳转生命周期；**禁止**逐代、逐帧、逐格
  或棋盘/RLE/窗口几何全文写入。
- 日志消息为英文 ASCII，便于 grep 与附在 bug report 中；用户提示仍为中文。
- Domain 不引入 Qt 日志；RLE 错误在 Service/Controller 边界记录。

### 6.3 并发

- 只有 `SimulationEngine` 与 Model 的跳转任务使用线程池。
- 后台任务只读快照、只写自己的输出缓冲区；UI 线程不等待计算（跳转的 `waitForFinished`
  仅用于析构）。
- 编辑/暂停/规则变化通过 `discardPending()` 使旧结果失效，不做跨线程共享写。
- Model 的信号只在 UI 线程发出；后台进度用 `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`。

### 6.4 尺寸与性能

- 棋盘上限 400×400，`Cell` 为 1 字节，`std::array` 连续存储。
- `Grid::resize()` 仅在尺寸变化时清零；同尺寸重复使用不 memset。
- Model 缓存人口，`stats()` O(1)。
- View 直接读 `const Grid&`，不逐帧复制。
- 导出 PNG 使用独立、受限的分辨率（不跟随 1–48 px/格缩放）。

### 6.5 View 只读约束

`BoardView` / `ControlPanel` 持有 `const model::LifeModel*`。C++ 的 const 限定会把
"View 修改 Model"变成编译错误；这是本项目最重要的编译期约束。

## 7. 构建与门禁

- CMake 目标：`lifegame_domain`、`lifegame_model`、`lifegame_service`、`lifegame_view`、
  `lifegame_controller`、`lifegame_app`、`lifegame`。
- `tools/check_layering.py` 校验 include 方向、`src/`/`..` 绕过、Domain/Model 的
  Qt/中文纯净度、`CMakeLists.txt` 源文件清单与 `target_link_libraries` 方向；
  `ctest` 中的 `layering` 用例失败即视为架构破坏。
- `tools/layering_selftest.py` 用负向 fixture 证明门禁能拒绝上述绕过；它也注册为 CTest。
- CI 至少执行：配置、构建（`-Wall -Wextra -Wpedantic`、默认 `-fno-exceptions`）、
  完整 `ctest`、无头 UI 测试。
- 新增文件必须放入正确的层目录，并在 `CMakeLists.txt` 中显式列出（禁止 glob）。

## 8. 评审清单

- [ ] 新增的 include 是否只指向本层或允许的下层？
- [ ] View 是否只持有 `const LifeModel*`，且没有调用写方法？
- [ ] Model 是否出现 `QFile`/`QSettings`/`QFileDialog`/中文文案/View 指针？
- [ ] Controller 是否只做输入编排，没有把棋盘复制给 View？
- [ ] Domain 是否仍然不包含任何 Qt 头文件？
- [ ] 新增状态变化是否通过信号广播，而不是让上层轮询？
- [ ] 可恢复失败是否返回错误码，而不是抛异常或只返回 bool？
- [ ] 是否在低频、可行动的路径打日志，且没有逐代/逐格/正文日志？
- [ ] 是否有对应的单元测试（Domain/Model）或 QtTest（View/Controller）？
