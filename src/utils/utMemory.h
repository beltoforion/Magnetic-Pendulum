#ifndef UT_MEMORY_H
#define UT_MEMORY_H

//--- Standard includes -----------------------------------------------------
#include <memory.h>  // for memset
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

  //---------------------------------------------------------------------------
  /** \brief Sets memory to zero. */
  template<typename TArray, std::size_t count>
  void memzero(TArray (&arr)[count])
  {
    std::memset( arr, 0, count*sizeof(TArray) );
  }

  //---------------------------------------------------------------------------
  template<typename TCont>
  void clear_cont_of_ptr(TCont &buf)
  {
    TCont::iterator item(buf.begin());
    for (; item!=buf.end(); ++item)
      delete (*item);

    buf.clear();
  }

}

#endif