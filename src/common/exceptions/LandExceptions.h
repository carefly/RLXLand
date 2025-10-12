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

class LandOutOfRangeException : public std::runtime_error {
public:
    explicit LandOutOfRangeException(const std::string& message) : std::runtime_error(message) {}
};

class LandPermissionException : public std::runtime_error {
public:
    explicit LandPermissionException(const std::string& message) : std::runtime_error(message) {}
};

class LandConflictException : public std::runtime_error {
public:
    explicit LandConflictException(const std::string& message) : std::runtime_error(message) {}
};

class TownOutOfRangeException : public std::runtime_error {
public:
    explicit TownOutOfRangeException(const std::string& message) : std::runtime_error(message) {}
};

class TownConflictException : public std::runtime_error {
public:
    explicit TownConflictException(const std::string& message) : std::runtime_error(message) {}
};

class TownPermissionException : public std::runtime_error {
public:
    explicit TownPermissionException(const std::string& message) : std::runtime_error(message) {}
};