#include <exception>
#include <string>

class Exception : public std::exception
{
   std::string msg_;
public:
   Exception(std::string msg) : msg_(msg) {}
   ~Exception() throw () {} // Updated
   const char* what() const throw() { return msg_.c_str(); }
};

