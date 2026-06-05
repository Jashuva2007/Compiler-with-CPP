#pragma once
#include <variant>
#include <string>
#include <cstdint>
#include <iostream>

using Value = std::variant<int64_t, bool>;

inline bool isInt (const Value& v) { return std::holds_alternative<int64_t>(v); }
inline bool isBool(const Value& v) { return std::holds_alternative<bool>(v); }
inline int64_t asInt (const Value& v) { return std::get<int64_t>(v); }
inline bool    asBool(const Value& v) { return std::get<bool>(v); }

inline std::string valueToString(const Value& v) {
    if (isInt(v))  return std::to_string(asInt(v));
    if (isBool(v)) return asBool(v) ? "true" : "false";
    return "<unknown>";
}