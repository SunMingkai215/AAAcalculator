# AAAcalculator

一个基于递归下降解析（recursive-descent parsing）实现的交互式表达式计算器（REPL）。

该项目使用 C++14 编写，作为解析器与表达式求值的教学示例，并提供如下功能：变量、常量、数学函数、幂运算、比较与逻辑运算等。

## 主要特性

- 支持整数与浮点数（例如 `42`, `3.14`）
- 支持算术运算：`+`, `-`, `*`, `/`, `%`
- 支持幂运算：`^` 或 `**`（右结合，如 `2 ^ 3 ^ 2 == 2 ^ (3 ^ 2)`）
- 支持一元运算与括号：`+`, `-`, `(`, `)`
- 支持自定义变量：`x = 2 + 3`（会话内可复用）
- 内置常量：`pi`, `e`（大小写不敏感）
- 内置函数（大小写不敏感）：`sin, cos, tan, sqrt, abs, log, ln, exp`，调用形式 `f(expr)`
- 支持比较运算：`>`, `<`, `>=`, `<=`（结果为 `1` 表示真，`0` 表示假）
- 支持逻辑运算：`!`（非），`&&`（与），`||`（或），逻辑结果同样为 `1`/`0`
- 简单错误报告（语法错误、域错误、除零等）

## 先决条件

- 支持 C++14 的编译器（例如 g++ 或 Visual Studio）
- 项目源文件：`AAAcalculator.cpp`

## 构建（示例）

使用 g++：

```
g++ -std=c++14 -O2 -o AAAcalculator AAAcalculator.cpp
./AAAcalculator
```

在 Visual Studio 中：打开或创建项目并添加源文件后构建运行。

## 使用（REPL）

启动程序后，在提示符下输入表达式或赋值语句：

- 退出：输入 `q` 或 `quit`
- 示例：
```
> x = 2 + 3
5
> x * 4
20
> sin(pi / 2)
1
> 2 ** 3 ^ 2
512  # 解析为 2 ** (3 ^ 2)
> a = 5 > 3
1
> !(a && (2 < 3))
0
```

说明：比较与逻辑运算的操作数为数值型；非零视为真，零视为假；运算结果使用 `1.0`（真）和 `0.0`（假）表示。

注意：本实现对链式比较（例如 `1 < 2 < 3`）按解析器普通方式处理（等同于 `(1 < 2) < 3`），这可能与数学上的链式比较语义不同。

## 表达式语法（简化 EBNF）

- Expression = Term { (`+` | `-`) Term } ; 加减
- Term = Power { (`*` | `/` | `%`) Power } ; 乘除取模
- Power = Factor { (`^` Power) | (`**` Power) } ; 右结合幂运算
- Factor = NUMBER | IDENTIFIER | IDENTIFIER `(` Expression `)` | `(` Expression `)` | (`+` | `-`) Factor
- Comparison = Expression { (`>` | `<` | `>=` | `<=`) Expression }
- LogicalNot = `!` LogicalNot | Comparison
- LogicalAnd = LogicalNot { `&&` LogicalNot }
- LogicalOr = LogicalAnd { `||` LogicalAnd }

最终解析入口为 `LogicalOr`，支持逻辑、比较与算术的组合表达式。

## 错误处理

- 语法错误时，程序会打印异常信息并继续 REPL
- 域错误示例：对负数调用 `sqrt`、对非正数调用 `log` 会报错
- 除零 / 取模零会报错

## 测试建议

- 覆盖优先级与结合性（算术、幂、比较、逻辑）测试
- 变量赋值与查找测试
- 内置函数与常量测试
- 错误路径测试（语法错误、域错误、除零）

可使用 C++ 单元测试框架（如 GoogleTest）进行自动化测试。

## 贡献

欢迎通过 issue 或 PR 贡献：

1. Fork 仓库并新建分支（`git checkout -b feature/your-feature`）
2. 提交修改并推送到你的分支
3. 提交 Pull Request，描述改动与测试步骤

## 许可证

本项目建议使用 MIT 许可证（如需，请添加 `LICENSE` 文件）。

---

README 遵循 Google 风格：简介、构建、使用、设计与行为说明，便于他人快速理解与上手。