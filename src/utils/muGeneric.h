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
  /** \brief PI as a double precision constant. */
  const double PI = 3.14159265358979323846;

  /** \brief Degree to radion conversion factor. */
  const double DEG2RAD = PI / 180.0f;

  //-------------------------------------------------------------------------------------------
  /** \brief Square root. */
  template<typename TVal>
  inline TVal sqr(const TVal &val)
  {
    return val * val;
  }

  //-------------------------------------------------------------------------------------------
  /** \brief Cubic root. */
  template<typename TVal>
  inline TVal qubic(const TVal &val)
  {
    return val * val * val;
  }

  //-------------------------------------------------------------------------------------------
  /** \brief Round to next integer. */
  template<typename TVal>
  inline int round(const TVal &val)
  {
    return (int)(val+0.5);
  }
}  // end of namespace MathUtils

#endif
