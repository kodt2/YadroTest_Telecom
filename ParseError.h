#pragma once
#include <iostream>
#include <fstream>
#include <string>

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& line, const std::string& what)
        : std::runtime_error(what), error_line(line) {}
    std::string error_line;
};
