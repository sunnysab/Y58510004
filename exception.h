//
// Created by sunnysab on 3/21/24.
//

#ifndef PARALLEL_EXCEPTION_H
#define PARALLEL_EXCEPTION_H

#include <stdexcept>
#include <string>

class Exception : public std::exception {
private:
    std::string message;

public:
    explicit Exception(const char* cstr)
            : message(cstr) {}

    explicit Exception(const std::string& str)
            : message(str) {}

    Exception(const Exception& other)
            : message(other.message) {}

    virtual ~Exception() {}

    virtual const char* what() const throw() {
        return message.c_str();
    }
};

#endif //PARALLEL_EXCEPTION_H
