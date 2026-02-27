#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <cmath>
#include <unordered_map>
#include <algorithm>

//一个用来跳过空格的辅助函数，可以让使用者得到更好的输入体验
static void skipSpaces(const std::string& s, size_t& pos) {
    while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) ++pos;
}
//转换数字的函数，支持整数和小数，使用stod进行转换，并且在转换失败时抛出异常
static double parseNumber(const std::string& s, size_t& pos) {
    skipSpaces(s, pos);
    size_t start = pos;
    bool hasDigits = false;
    while (pos < s.size() && (std::isdigit(static_cast<unsigned char>(s[pos])))) { ++pos; hasDigits = true; }
    if (pos < s.size() && s[pos] == '.') {
        ++pos;
        while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) { ++pos; hasDigits = true; }
    }
    if (!hasDigits) throw std::runtime_error("Expected number");
    std::string token = s.substr(start, pos - start);
    try {
        return std::stod(token);
    }
    catch (...) {
        throw std::runtime_error("Invalid number: " + token);
    }
}

//截取变量名的函数，变量名必须以字母或下划线开头
static std::string parseIdentifier(const std::string& s, size_t& pos) {
    skipSpaces(s, pos);
    if (pos >= s.size()) return "";
    if (!(std::isalpha(static_cast<unsigned char>(s[pos])) || s[pos] == '_')) return "";//如果没有用字母开头或者下划线开头就返回空
    size_t start = pos;
    ++pos;
    while (pos < s.size() && (std::isalnum(static_cast<unsigned char>(s[pos])) || s[pos] == '_')) ++pos;
    return s.substr(start, pos - start);
}
//提前声明
static std::unordered_map<std::string, double> variables;
static double parseExpression(const std::string& s, size_t& pos);
static double parsePower(const std::string& s, size_t& pos);
static double parseComparison(const std::string& s, size_t& pos);
static double parseLogicalNot(const std::string& s, size_t& pos);
static double parseLogicalAnd(const std::string& s, size_t& pos);
static double parseLogicalOr(const std::string& s, size_t& pos);

//统一转小写，方便对于内置的参数和函数不区分大小写
static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

//处理数字和括号以及函数调用的函数，支持正负号，括号内的表达式，以及函数调用
static double parseFactor(const std::string& s, size_t& pos) {
    skipSpaces(s, pos);
    if (pos >= s.size()) throw std::runtime_error("Unexpected end of expression");

    if (s[pos] == '+') {
        ++pos;
        return parseFactor(s, pos);
    }
    if (s[pos] == '-') {
        ++pos;
        return -parseFactor(s, pos);
    }
    if (s[pos] == '(') {
        ++pos;
        double val = parseExpression(s, pos);
        skipSpaces(s, pos);
        if (pos >= s.size() || s[pos] != ')') throw std::runtime_error("Missing closing parenthesis");
        ++pos;
        return val;
    }
    //识别变量或函数关键字
    if (std::isalpha(static_cast<unsigned char>(s[pos])) || s[pos] == '_') {
        //把单词截取出来，先判断是否是函数调用，如果是函数调用则继续解析括号内的表达式作为参数，如果不是函数调用则判断是否是常量或变量
        std::string id = parseIdentifier(s, pos);
        std::string idlow = toLower(id);
        skipSpaces(s, pos);
        //有可能会遇到括号，那样的话处理方式和普通括号一样，先解析括号内的表达式作为参数，然后根据函数名调用对应的函数
        if (pos < s.size() && s[pos] == '(') {
            ++pos; 
            double arg = parseExpression(s, pos);
            skipSpaces(s, pos);
            if (pos >= s.size() || s[pos] != ')') throw std::runtime_error("Missing closing parenthesis after function argument");
            ++pos; //必须检验括号右边有没有封上，有的话就指针后移跳过这个右括号
            if (idlow == "sin") return std::sin(arg);
            if (idlow == "cos") return std::cos(arg);
            if (idlow == "tan") return std::tan(arg);
            if (idlow == "sqrt") {
                if (arg < 0) throw std::runtime_error("sqrt domain error");
                return std::sqrt(arg);
            }
            if (idlow == "abs") return std::fabs(arg);
            if (idlow == "log" || idlow == "ln" || idlow == "in") {
                if (arg <= 0) throw std::runtime_error("log domain error");
                return std::log(arg);
            }
            if (idlow == "exp") return std::exp(arg);
            // 如果是未知函数就抛出异常
            throw std::runtime_error("Unknown function: " + id);
        }
        // 在检验是否是函数调用之后再来判断是否是常量或变量，这样就不会把函数名当成变量来处理了
        if (idlow == "pi") return std::acos(-1.0);
        if (idlow == "e") return std::exp(1.0);
        auto it = variables.find(id);
        if (it != variables.end()) return it->second; // 找到变量直接返回值，证明这个变量是定义过的，直接return，不然就抛出异常了
        throw std::runtime_error("Undefined identifier: " + id);
    }
    return parseNumber(s, pos);
}

// 处理指数运算的函数，支持右结合的幂运算符^和**，并且在遇到幂运算符时调用parsePower来处理右边的表达式，这样就能右结合
static double parsePower(const std::string& s, size_t& pos) {
    double left = parseFactor(s, pos);
    while (true) {
        skipSpaces(s, pos);
        if (pos >= s.size()) break;
        // Check for '^' or '**'
        if (s[pos] == '^') {
            ++pos;
            double rhs = parsePower(s, pos); // right-associative
            left = std::pow(left, rhs);
            continue;
        }
        if (s[pos] == '*' && pos + 1 < s.size() && s[pos + 1] == '*') {
            pos += 2;
            double rhs = parsePower(s, pos);
            left = std::pow(left, rhs);
            continue;
        }
        break;
    }
    return left;
}

static double parseTerm(const std::string& s, size_t& pos) {
    double value = parsePower(s, pos);
    while (true) {
        skipSpaces(s, pos);
        if (pos >= s.size()) break;
        if (s[pos] == '*' && pos + 1 < s.size() && s[pos + 1] == '*') break;
        char op = s[pos];
        if (op != '*' && op != '/' && op != '%') break;
        ++pos;
        double rhs = parsePower(s, pos);
        if (op == '*') {
            value = value * rhs;
        }
        else if (op == '/') {
            if (std::abs(rhs) < 1e-15) throw std::runtime_error("Division by zero");
            value = value / rhs;
        }
        else {
            if (std::abs(rhs) < 1e-15) throw std::runtime_error("Modulo by zero");
            double iv1 = std::round(value);
            double iv2 = std::round(rhs);
            if (std::abs(value - iv1) < 1e-12 && std::abs(rhs - iv2) < 1e-12) {
                long long a = static_cast<long long>(iv1);
                long long b = static_cast<long long>(iv2);
                value = static_cast<double>(a % b);
            }
            else {
                value = std::fmod(value, rhs);
            }
        }
    }
    return value;
}

static double parseExpression(const std::string& s, size_t& pos) {
    double value = parseTerm(s, pos);
    while (true) {
        skipSpaces(s, pos);
        if (pos >= s.size()) break;
        char op = s[pos];
        if (op != '+' && op != '-') break;
        ++pos;
        double rhs = parseTerm(s, pos);
        if (op == '+') value = value + rhs; else value = value - rhs;
    }
    return value;
}

// 处理比较运算：<, >, <=, >=，注意：尽量不要使用链式比较，本计算器的链式比较是解析器那种而非数学上的链式比较
static double parseComparison(const std::string& s, size_t& pos) {
    double left = parseExpression(s, pos);
    while (true) {
        skipSpaces(s, pos);
        if (pos >= s.size()) break;
        // 先处理 >= 和 <= 
        if (s[pos] == '>' && pos + 1 < s.size() && s[pos + 1] == '=') {
            pos += 2;
            double rhs = parseExpression(s, pos);
            left = (left >= rhs) ? 1.0 : 0.0;
            continue;
        }
        if (s[pos] == '<' && pos + 1 < s.size() && s[pos + 1] == '=') {
            pos += 2;
            double rhs = parseExpression(s, pos);
            left = (left <= rhs) ? 1.0 : 0.0;
            continue;
        }
        if (s[pos] == '>') {
            ++pos;
            double rhs = parseExpression(s, pos);
            left = (left > rhs) ? 1.0 : 0.0;
            continue;
        }
        if (s[pos] == '<') {
            ++pos;
            double rhs = parseExpression(s, pos);
            left = (left < rhs) ? 1.0 : 0.0;
            continue;
        }
        break;
    }
    return left;
}

// 逻辑非：'!'，对非零值返回0，对零返回1
static double parseLogicalNot(const std::string& s, size_t& pos) {
    skipSpaces(s, pos);
    if (pos < s.size() && s[pos] == '!') {
        ++pos;
        double v = parseLogicalNot(s, pos);
        return (std::abs(v) < 1e-15) ? 1.0 : 0.0;
    }
    return parseComparison(s, pos);
}

// 逻辑与：'&&'，按从左至右结合
static double parseLogicalAnd(const std::string& s, size_t& pos) {
    double left = parseLogicalNot(s, pos);
    while (true) {
        skipSpaces(s, pos);
        if (pos + 1 < s.size() && s[pos] == '&' && s[pos + 1] == '&') {
            pos += 2;
            double rhs = parseLogicalNot(s, pos);
            left = (std::abs(left) > 1e-15 && std::abs(rhs) > 1e-15) ? 1.0 : 0.0;
            continue;
        }
        break;
    }
    return left;
}

// 逻辑或：'||'，按从左至右结合
static double parseLogicalOr(const std::string& s, size_t& pos) {
    double left = parseLogicalAnd(s, pos);
    while (true) {
        skipSpaces(s, pos);
        if (pos + 1 < s.size() && s[pos] == '|' && s[pos + 1] == '|') {
            pos += 2;
            double rhs = parseLogicalAnd(s, pos);
            left = (std::abs(left) > 1e-15 || std::abs(rhs) > 1e-15) ? 1.0 : 0.0;
            continue;
        }
        break;
    }
    return left;
}

static double evaluate(const std::string& input) {
    size_t pos = 0;
    skipSpaces(input, pos);

    //先截取变量名再进行值的处理
    size_t save = pos;
    std::string name = parseIdentifier(input, pos);
    if (!name.empty()) {
        size_t tmp = pos;
        skipSpaces(input, tmp);
        if (tmp < input.size() && input[tmp] == '=') {
            tmp++; //跳过=指向后面的数字
            double rhs = parseLogicalOr(input, tmp);
            skipSpaces(input, tmp);
            if (tmp != input.size()) throw std::runtime_error("Unexpected character at position " + std::to_string(tmp));
            variables[name] = rhs;
            return rhs;
        }
    }
    //正常运算
    pos = 0;
    double result = parseLogicalOr(input, pos);
    skipSpaces(input, pos);
    if (pos != input.size()) throw std::runtime_error("Unexpected character at position " + std::to_string(pos));
    return result;
}

int main() {
    std::cout << "Simple calculator. Enter expressions to evaluate. Type 'q' or 'quit' to exit." << std::endl;
    std::cout << "Supports variables (x = 1), constants (pi, e) and functions: sin, cos, tan, sqrt, abs, log, ln, exp." << std::endl;
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;
        size_t p = 0; skipSpaces(line, p);
        std::string token = line.substr(p);
        if (token == "q" || token == "quit") break;
        try {
            double res = evaluate(line);
            double rround = std::round(res);
            if (std::abs(res - rround) < 1e-12) {
                std::cout << static_cast<long long>(rround) << std::endl;
            }
            else {
                std::cout.setf(std::ios::fmtflags(0), std::ios::floatfield);//让浮点数按照实际有效输出格式输出
                std::cout << res << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cout << "Error: " << ex.what() << std::endl;
        }
    }
    return 0;
}