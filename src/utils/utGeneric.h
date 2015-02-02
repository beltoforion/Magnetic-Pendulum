#ifndef UT_GENERIC_H
#define UT_GENERIC_H

//--- Standard includes -----------------------------------------------------
#include <cstddef>  // for std::size_t

namespace utils
{
  //---------------------------------------------------------------------------
  /** \brief Return the size of an array.
  */
  template <typename TArray, std::size_t count>
  inline std::size_t count_of(TArray (&)[count])
  {
    return count;
  } 
}

#endif