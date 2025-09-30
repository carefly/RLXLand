#pragma once
#include <stdexcept>
#include <string>

class PlayerNotFoundException : public std::runtime_error {
public:
    explicit PlayerNotFoundException(const std::string& message) : std::runtime_error(message) {}
};

class LandNotFoundException : public std::runtime_error {
public:
    explicit LandNotFoundException(const std::string& message) : std::runtime_error(message) {}
};

class DuplicateException : public std::runtime_error {
public:
    explicit DuplicateException(const std::string& message) : std::runtime_error(message) {}
};

class NotMemberException : public std::runtime_error {
public:
    explicit NotMemberException(const std::string& message) : std::runtime_error(message) {}
};