/** \file Generic math utility classes
    \author Ingo Berg

    Last changed:  02.10.2006

    Modification history:
    - 02.10.2006 Initial version
*/
#ifndef MU_GENERIC_H
#define MU_GENERIC_H

namespace mu
{
    const double PI = 3.14159265358979323846;

    const double DEG2RAD = PI / 180.0f;

    template<typename TVal>
    inline TVal sqr(const TVal &val)
    {
        return val * val;
    }

    template<typename TVal>
    inline TVal qubic(const TVal &val)
    {
        return val * val * val;
    }

    template<typename TVal>
    inline TVal pow4(const TVal& val)
    {
        return val * val * val * val;
    }

    template<typename TVal>
    inline int round(const TVal &val)
    {
        return (int)(val + 0.5);
    }
}  // end of namespace MathUtils

#endif
