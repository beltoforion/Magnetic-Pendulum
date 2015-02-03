#ifndef SU_UTILITY_H
#define SU_UTILITY_H

#include <algorithm>
#include <string>

/*!
    \brief Supplementary classes to be used mostly in combination with STL objects.

    This namespace contains elements that mostly depend on STL objects. It's intention is
    to provide an archive of small helper classes, functions and function objects that
    are closely tied to the C++ standard library. Some of the classes are written by me,
    others have been taken from books or the internet.

    \warning
    Changes and additions of functions that are project specific are not allowed here!
 */
namespace su
{
  //--------------------------------------------------------------------------
  template<typename TStr>
  TStr trim_left (const TStr &a_str, const TStr &a_chars)
  {
    const TStr::size_type np = a_str.find_first_not_of(a_chars);
    return (np == TStr::npos) ? a_str : a_str.substr(np);
  }

  //--------------------------------------------------------------------------
  template<typename TStr>
  TStr trim_right (const TStr &a_str, const TStr &a_chars)
  {
    const TStr::size_type np = a_str.find_last_not_of(a_chars);
    return (np == TStr::npos) ? a_str : a_str.substr(0, np + 1);
  }

  //--------------------------------------------------------------------------
  template<typename TStr>
  TStr trim(const TStr &a_str, const TStr &a_chars = TStr(_T(" ")) )
  {
    return (a_str.empty()) ? a_str : trim_right(trim_left(a_str, a_chars), a_chars);
  }

  //--------------------------------------------------------------------------
  template<typename TStr>
  void replace_all( TStr &a_str, 
                    typename TStr::value_type a_chOld,
                    typename TStr::value_type a_chNew )
  {
      for (std::size_t i=0; i<a_str.length(); ++i)
          a_str[i] = (a_str[i]==a_chOld) ? a_chNew : a_str[i];
  }

  //--------------------------------------------------------------------------
  /** \brief Remove quotes from a string. 

    Does nothing, if no quotes are found.
  */
  template<typename TStr>
  TStr unquote(TStr &a_str)
  {
    TStr  buf(a_str);
    TStr::size_type size(1);

    while(size!=TStr::npos)
    {
      size = buf.find_first_of('"');
      if (size!=TStr::npos)
        buf.erase(size, 1);
    }

    a_str = buf;

    return buf;
  } 

  //-------------------------------------------------------------------------------------
  template<typename TStr>
  TStr to_upper(TStr a_sInput)
  {
    TStr sOutput(a_sInput);
    int (*pf)(int) = toupper; 
    std::transform(sOutput.begin(), sOutput.end(), sOutput.begin(), pf); 

    return sOutput;
  } 

}  // end of namespace string utils

#endif
