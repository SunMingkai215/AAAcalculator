# AAAcalculator — 设计文档

作者: 自动生成（可编辑）
日期: 2026-02-27
版本: 1.0

## 1. 目的与范围

本文档描述 `AAAcalculator` 项目的设计与实现要点，目标读者为实现者、维护者与希望扩展功能的贡献者。文档覆盖：总体架构、解析器文法、模块接口、主要算法、错误处理、测试计划与可扩展性建议。

不覆盖部署细节（本项目为本地命令行工具）与性能调优的深度剖析。

## 2. 背景与约束

- 语言与标准：C++14
- 单文件实现：`AAAcalculator.cpp`（当前实现）
- 运行环境：命令行 REPL，单线程
- 约束：使用递归下降解析器实现表达式求值，设计尽量保持代码结构简单以便教学与扩展。

## 3. 概览（架构）

高层组件：

- REPL（`main`）
  - 负责读取输入、处理退出命令、打印结果与错误信息。
- 解析与求值模块（递归下降解析器）
  - 词法/标识：`skipSpaces`、`parseNumber`、`parseIdentifier`
  - 语法/解析：`parseFactor`, `parsePower`, `parseTerm`, `parseExpression`, `parseComparison`, `parseLogicalNot`, `parseLogicalAnd`, `parseLogicalOr`
  - 顶层入口：`evaluate`（处理赋值并调用解析入口）
- 运行时环境
  - 变量表：`std::unordered_map<std::string,double> variables` 存储会话级变量
  - 内置常量/函数：在 `parseFactor` 中实现（例如 `pi`, `e`, `sin`, `cos` 等）

数据流：REPL -> evaluate -> 解析器层次（逻辑 -> 比较 -> 算术）-> 变量查找/函数计算 -> 返回结果 -> REPL 输出。

## 4. 文法（与代码对应）

使用简化 EBNF 表示：

- LogicalOr = LogicalAnd { `||` LogicalAnd }
- LogicalAnd = LogicalNot { `&&` LogicalNot }
- LogicalNot = `!` LogicalNot | Comparison
- Comparison = Expression { (`>` | `<` | `>=` | `<=`) Expression }
- Expression = Term { (`+` | `-`) Term }
- Term = Power { (`*` | `/` | `%`) Power }
- Power = Factor { (`^` Power) | (`**` Power) }  ; 右结合
- Factor = NUMBER | IDENTIFIER | IDENTIFIER `(` Expression `)` | `(` Expression `)` | (`+` | `-`) Factor

代码映射：

- `LogicalOr` -> `parseLogicalOr`
- `LogicalAnd` -> `parseLogicalAnd`
- `LogicalNot` -> `parseLogicalNot`
- `Comparison` -> `parseComparison`
- `Expression` -> `parseExpression`
- `Term` -> `parseTerm`
- `Power` -> `parsePower`
- `Factor` -> `parseFactor`

备注：解析入口由 `evaluate` 调用 `parseLogicalOr`，因此解析器按逻辑运算优先外层逐层进入算术运算。

## 5. 模块与接口

主要函数（摘要）：

- `double evaluate(const std::string &input)`
  - 处理赋值语句（`name = expr`），否则调用 `parseLogicalOr` 评估表达式。
  - 返回计算结果（`double`），布尔值使用 `1.0`/`0.0` 表示。

- 解析子函数（均为 `static double func(const std::string&, size_t&)`）
  - `parseFactor`：数字、变量、内置常量、函数调用、括号与一元正负。
  - `parsePower`：实现 `^`、`**` 右结合。
  - `parseTerm`：处理 `*`, `/`, `%`。
  - `parseExpression`：处理 `+`, `-`。
  - `parseComparison`：处理 `>`, `<`, `>=`, `<=`。
  - `parseLogicalNot` / `parseLogicalAnd` / `parseLogicalOr`：处理逻辑运算 `!`, `&&`, `||`。

- 词法辅助：
  - `skipSpaces`, `parseNumber`, `parseIdentifier`。

数据与全局：

- `std::unordered_map<std::string,double> variables`：会话级变量表。

## 6. 关键算法与实现要点

- 递归下降解析：每个非终结符对应一个解析函数，函数通过递归表达式实现右结合或左结合。

- 幂运算的右结合实现：在 `parsePower` 中，当识别到 `^` 或 `**` 时，使用递归调用 `parsePower` 来解析右侧操作数，从而实现右结合语义。

- 函数调用解析：在 `parseFactor` 中，当标识符后跟 `(` 时，解析括号内 `Expression` 作为单个参数，并在解析后调用对应的数学函数（`sin`, `cos`, `sqrt` 等）。目前只支持单参数函数。

- 变量与常量：标识符解析后，优先检测内置常量（`pi`, `e`），否则在 `variables` 中查找；若未定义抛出异常。

- 布尔与逻辑：逻辑值在内部为 `double` 类型，非零视为真。逻辑运算使用数值为真或假的判断（阈值 `1e-15`），返回 `1.0` 或 `0.0`。

- 错误报告：通过抛出 `std::runtime_error` 异常携带错误信息，REPL 在 `main` 中捕获并打印。

## 7. 错误处理与边界条件

- 非法字符或语法不完整（如缺失右括号、期望数字但遇到字母）会抛出明确的错误字符串。
- 数学域错误：`sqrt`（负数）、`log`（非正数）在解析时检测并抛错。
- 除零/取模零会检测并抛出错误。
- 标识符未定义会报 `Undefined identifier`。
- 浮点比较：比较与逻辑判断使用绝对阈值判断是否为零（目前硬编码 `1e-15`）；可按需求改为相对误差或可配置阈值。

## 8. 测试计划

推荐使用单元测试框架（如 GoogleTest）实现以下测试用例：

1. 基本算术与优先级
   - `1 + 2 * 3 == 7`
   - `(1 + 2) * 3 == 9`
2. 幂运算与结合性
   - `2 ^ 3 ^ 2 == 2 ^ (3 ^ 2)`
   - `2 ** 3 == 8`
3. 变量赋值与使用
   - `x = 5` 后 `x * 2 == 10`
   - 未定义 `y` 报错
4. 内置函数与常量
   - `sin(pi / 2) == 1`
   - `sqrt(4) == 2`
5. 比较与逻辑
   - `5 > 3 == 1`
   - `!(0) == 1`、`1 && 0 == 0`
6. 错误路径
   - `sqrt(-1)` 报错
   - `1 / 0` 报错