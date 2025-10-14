#pragma once
#include <stdexcept>
#include <string>

class PlayerNotFoundException : public std::runtime_error {
public:
    explicit PlayerNotFoundException(const std::string& message) : std::runtime_error(message) {}
};

class RealmNotFoundException : public std::runtime_error {
public:
    explicit RealmNotFoundException(const std::string& message) : std::runtime_error(message) {}
};

class DuplicateException : public std::runtime_error {
public:
    explicit DuplicateException(const std::string& message) : std::runtime_error(message) {}
};

class NotMemberException : public std::runtime_error {
public:
    explicit NotMemberException(const std::string& message) : std::runtime_error(message) {}
};

class RealmOutOfRangeException : public std::runtime_error {
public:
    explicit RealmOutOfRangeException(const std::string& message) : std::runtime_error(message) {}
};

class RealmPermissionException : public std::runtime_error {
public:
    explicit RealmPermissionException(const std::string& message) : std::runtime_error(message) {}
};

class RealmConflictException : public std::runtime_error {
public:
    explicit RealmConflictException(const std::string& message) : std::runtime_error(message) {}
};

class InvalidCoordinatesException : public std::runtime_error {
public:
    explicit InvalidCoordinatesException(const std::string& message) : std::runtime_error(message) {}
};

class InvalidPlayerInfoException : public std::runtime_error {
public:
    explicit InvalidPlayerInfoException(const std::string& message) : std::runtime_error(message) {}
};
