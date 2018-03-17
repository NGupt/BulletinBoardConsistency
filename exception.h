#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <iostream>

class Exception : public std::exception {
public:
    std::string message;
    Exception(std::string message) : message(message) {}
    ~Exception() throw() {}
    const char *what() const throw() { return message.c_str(); }

};

#endif // EXCEPTION_H
