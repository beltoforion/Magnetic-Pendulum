#ifndef _UT_WRUNTIME_ERROR
#define _UT_WRUNTIME_ERROR

#include <stdexcept>
#include <string>

namespace utils
{
    class wruntime_error : public std::runtime_error
    {
    public:
        wruntime_error(const std::wstring& errorMsg);
        wruntime_error(const wruntime_error& rhs);
        wruntime_error& operator=(const wruntime_error& rhs);
        virtual ~wruntime_error();

        const std::wstring& message() const;

    private:
        std::wstring _message;
    };
}

#endif