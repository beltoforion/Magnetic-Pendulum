#ifndef StdUtilsUtilityH
#define StdUtilsUtilityH

#include <algorithm>


namespace MathUtils
{

//--------------------------------------------------------------------------
/// \brief Function template that cuts the value passed as the first parameter if
///        it exceeds the min and max limits passed as second and third parameter.
///
/// The function will test if a_minLimit<a_maxLimit swapping both values if necessary.
/// \param a_minLimit The minimum limit.
/// \param a_maxLimit The maximum limit.
template<typename TData>
inline TData &LimitValue(TData &a_value, const TData &a_min, const TData &a_max)
{
  // Swap limits, if necessary
  const TData min = std::min(a_min, a_max);
  const TData max = std::max(a_min, a_max);

  return a_value = std::min(max, std::max(min, a_value));
  // limit to upper and lower bound
}

//--------------------------------------------------------------------------
/** Verify if a value lies in a distinct the percentage range of another value.
    \param a_range Percentage range [0..1]
    \param a_val1 Value to be range tested
    \param a_val2 Absolute value for range comparison
    \throw logic_error
 */
template<typename TData>
inline bool InRelRange(const TData &a_val1, const TData &a_val2, double a_range)
{
  TData DiffAbs = (a_range*a_val2);
  return InRange(a_val1, a_val2-DiffAbs, a_val2+DiffAbs);
}

//--------------------------------------------------------------------------
template<typename TData>
inline bool InRange(const TData &a_val, const TData &a_min, const TData &a_max)
{
  const double min = std::min(a_min, a_max);
  const double max = std::max(a_min, a_max);

  return (a_val>=min) && (a_val<=max);
}


}  // end of namespace MathUtils

#endif
