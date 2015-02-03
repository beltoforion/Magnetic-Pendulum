#include "utWideExceptions.h"

using namespace std;


namespace utils
{
    wruntime_error::wruntime_error(const std::wstring& msg)
        :std::runtime_error("unexpected exception")
        ,_message(msg)
    {}
    
    wruntime_error::wruntime_error(const wruntime_error& rhs)
        :std::runtime_error("unexpected exception")
        ,_message(rhs._message)
    {}

    wruntime_error& wruntime_error::operator=(const wruntime_error& rhs)
    {
        std::runtime_error::operator=(rhs);
        _message = rhs._message;
        return *this;
    }
        
    wruntime_error::~wruntime_error()
    {}

    const std::wstring& wruntime_error::message() const
    {
        return _message;
    }
}