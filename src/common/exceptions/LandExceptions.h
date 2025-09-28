#pragma once
#include <stdexcept>
#include <string>

class LandNotFoundException : public std::runtime_error {
public:
    explicit LandNotFoundException(const std::string& message) : std::runtime_error(message) {}
};

class LandDuplicateException : public std::runtime_error {
public:
    explicit LandDuplicateException(const std::string& message) : std::runtime_error(message) {}
};

class LandMemberException : public std::runtime_error {
public:
    explicit LandMemberException(const std::string& message) : std::runtime_error(message) {}
};