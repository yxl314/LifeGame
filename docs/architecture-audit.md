# 标准分层 MVC 独立审计

本审计不使用 `docs/architecture.md` 作为证据，而是直接检查源码 include、CMake target graph、
测试依赖和运行行为。采用的验收基准是**分层 MVC + 被动视图**：

- View 不知道 Model，不 include `model/`，不连接 Model 信号；
- Controller 是唯一同时持有可写 Model 与 View 的层（App 组合根除外）；
- Model/Service/View 只依赖 Domain/Shared，不互相依赖；
- Domain/Shared 不依赖 Qt，Model 不依赖 UI/文件/设置；
- CMake 链接方向与 include 方向一致；
- 有负向测试证明门禁会拒绝绕过。

## 审计结果

| 规则 | 证据 | 结果 |
|---|---|---|
| View 不 include Model | `grep -rn '#include "model/' src/view` 无输出 | 通过 |
| View 不持有 `LifeModel*`/不连 Model 信号 | `grep -rn 'LifeModel\|model::' src/view` 无输出 | 通过 |
| View 只依赖 Domain/Shared | `lifegame_view` 链接 `lifegame_domain`/`lifegame_shared` + Qt Widgets | 通过 |
| View 是被动 setter | `BoardView::setBoard/setPopulation/setRunning/refresh`；`ControlPanel::setStats/setRunning/setBoardSize/...` | 通过 |
| Controller 是唯一 Model↔View 中介 | `BoardController`/`ConfigurationController` 订阅 Model 信号后调用 View setter；View 信号只连 Controller | 通过 |
| Service 不依赖 Model | `grep -rn '#include "model/' src/service` 无输出；`lifegame_service` 只链接 Domain/Shared | 通过 |
| Model 不依赖 View/Controller/Service | `grep` 无输出；`lifegame_model` 只链接 Domain/Shared/QtCore/QtConcurrent | 通过 |
| Domain 不依赖 Qt | Domain target 不链接 Qt；源码无 `<Q...>` include | 通过 |
| Shared 只依赖自身 | `src/shared` 无任何层 include；无 Qt | 通过 |
| Controller 不依赖 App | `grep -rn '#include "app/' src/controller` 无输出 | 通过 |
| App 不直接命令 Model | `grep -rn 'm_model->' src/app` 无输出；App 只组合/转发 Controller | 通过 |
| 测试能脱离 Model 测 View | `view_tests` 只链接 `lifegame_view`；用 `domain::Grid` 构造棋盘 | 通过 |
| 门禁是否只是“文档 OK” | `layering_selftest` 负向 fixture：View→Model、Service→Model、Shared→Qt 全部失败 | 通过 |
| 目标图 | `lifegame_model/service/view → domain/shared`；`controller → 三者`；`app → controller` | 通过 |

## 源码级证据

```text
src/view  → domain, shared, QtWidgets
src/model → domain, shared, QtCore, QtConcurrent
src/service → domain, shared, QtCore, QtGui
src/controller → model, view, service, domain, shared, QtWidgets
src/app → controller
```

`tools/check_layering.py` 对以下项返回非零：

- View 或 Service include `model/`
- `src/` 前缀、`..`/绝对路径绕过
- Domain/Shared/Model 的 Qt 或中文污染
- Manifest 未列文件
- 反向 `target_link_libraries`

`tools/layering_selftest.py` 为上述规则构造临时源码树，确认门禁会失败，而不是只会在
正确代码上打印 `Layering OK`。

## 与“经典 Observer MVC”的区别

原实现属于经典 Observer MVC：View 直接持有 `const LifeModel*`、连接 Model 信号并读取
`model.board()`。这是合法 MVC，但**不是**分层 MVC 中“Controller 中介、View 被动”的变体。
本次重构后：

- View 不再知道 `LifeModel` 类型；
- Model 信号只由 Controller 消费；
- Controller 用 `setBoard(const Grid*)`/`setStats(LifeStats)` 等 setter 推送状态；
- 棋盘仍不复制：`BoardView` 只持有 `const domain::Grid*`。

## 仍需说明的设计选择

1. **Shared 层**：`LifeStats`/`AppSettings`/`JumpInfo` 是跨层 DTO，不是业务实体，因此放在
   `src/shared`，避免污染 Domain；`shared` 不依赖任何其他层。
2. **Model 使用 Qt signal/QtConcurrent**：这是 Qt 应用模型，不是纯领域实体；纯业务规则在
   `domain`，可脱离 Qt 测试。
3. **View 是具体类，不是接口**：Controller 直接调用具体 View 的 setter；这是桌面 Qt MVC
   的正常做法。`MessageSink`/`ConfirmSink`/`JumpProgressView` 三个输出端口保留接口化，
   便于测试确认/取消/进度。
4. **App 组合根**持有 Model/View 指针用于注入，但不直接调用 Model 命令；这是组合根例外，
   不是 Controller 职责泄漏。
5. **`SessionController` 承担启动恢复/退出保存**：它是 Controller 层的应用协调器；不是
   业务规则，也不绕过 Model/Service。

## 复核命令

```bash
cmake --build --preset debug --parallel 8
ctest --preset debug
python3 tools/check_layering.py .
python3 tools/layering_selftest.py
```

当前结果：8/8 CTest 通过；门禁 85 个文件通过；负向 fixture 全部按预期失败。
