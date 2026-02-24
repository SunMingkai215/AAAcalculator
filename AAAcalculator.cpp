#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <cmath>


static void skipSpaces(const std::string& s, size_t& pos) {
    while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) ++pos;
}

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

// Forward declarations
static double parseExpression(const std::string& s, size_t& pos);

// parseFactor: numbers, parenthesis, unary + and -
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
    return parseNumber(s, pos);
}

static double parseTerm(const std::string& s, size_t& pos) {
    double value = parseFactor(s, pos);
    while (true) {
        skipSpaces(s, pos);
        if (pos >= s.size()) break;
        char op = s[pos];
        if (op != '*' && op != '/' && op != '%') break;
        ++pos;
        double rhs = parseFactor(s, pos);
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

static double evaluate(const std::string& input) {
    size_t pos = 0;
    double result = parseExpression(input, pos);
    skipSpaces(input, pos);
    if (pos != input.size()) throw std::runtime_error("Unexpected character at position " + std::to_string(pos));
    return result;
}

int main() {
    std::cout << "Simple calculator. Enter expressions to evaluate. Type 'q' or 'quit' to exit." << std::endl;
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