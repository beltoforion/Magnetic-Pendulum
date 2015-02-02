#ifndef MU_VECTOR_H
#define MU_VECTOR_H

//--- Standard includes ---------------------------------------------------------------------
#include <memory.h>  // for memset
#include <cstdlib>
#include <cassert>

//-------------------------------------------------------------------------------------------
#include "utils/utMemory.h"


namespace mu
{
  //-------------------------------------------------------------------------------------------
  //
  // Forward declarations
  //
  template<typename TValType, int TDim>
  class Vector;

  //-------------------------------------------------------------------------------------------
  template<typename TValType, int TDim>
  TValType abs(const Vector<TValType, TDim> &v);

  //-------------------------------------------------------------------------------------------
  // Addition of two vectors
  template<typename TValType, int TDim>
  Vector<TValType, TDim> operator+( const Vector<TValType, TDim> &v1, 
                                    const Vector<TValType, TDim> &v2 );
  //-------------------------------------------------------------------------------------------
  // Subtraction of two vectors
  template<typename TValType, int TDim>
  Vector<TValType, TDim> operator-( const Vector<TValType, TDim> &v1, 
                                    const Vector<TValType, TDim> &v2 );

  //-------------------------------------------------------------------------------------------
  // Multiplication of two vectors element wise
  template<typename TValType, int TDim>
  Vector<TValType, TDim> operator*( const Vector<TValType, TDim> &v1, 
                                    const Vector<TValType, TDim> &v2 );

  //-------------------------------------------------------------------------------------------
  // Multiplication of two vectors element wise
  template<typename TValType, int TDim>
  Vector<TValType, TDim> operator/( const Vector<TValType, TDim> &v1, 
                                    const Vector<TValType, TDim> &v2 );

  //-------------------------------------------------------------------------------------------
  // Multiplication with a value 
  template<typename TValType, int TDim>
  Vector<TValType, TDim> operator*( const TValType &val, 
                                    const Vector<TValType, TDim> &v2 );
  template<typename TValType, int TDim>
  Vector<TValType, TDim> operator*( const Vector<TValType, TDim> &v2,
                                    const TValType &val );

  //-------------------------------------------------------------------------------------------
  /** \brief Fixed size vector. */
  template<typename TValType, int TDim>
  class Vector
  {
  template <typename TValType, int TDim>  
  friend TValType abs(const Vector& v);

  template <typename TValType, int TDim>  
  friend Vector operator+( const Vector&, const Vector& );

  template <typename TValType, int TDim>  
  friend Vector operator*( const Vector&, const Vector& );

  template <typename TValType, int TDim>  
  friend Vector operator/( const Vector&, const Vector& );

  template <typename TValType, int TDim>  
  friend Vector operator-( const Vector&, const Vector& );

  template<typename TValType, int TDim>
  friend Vector operator*( const TValType&, 
                           const Vector& );

  public:	
    typedef TValType value_type;

    //-----------------------------------------------------------------------------------------
    Vector()
    {
      utils::memzero(m_aData);
    }

    //-----------------------------------------------------------------------------------------
    Vector(value_type v1, value_type v2)
    {
      Assign(v1, v2);
    }

    //-----------------------------------------------------------------------------------------
    Vector(value_type v1, value_type v2, value_type v3)
    {
      assert(TDim==3);
      m_aData[0] = v1;
      m_aData[1] = v2;
      m_aData[2] = v3;
    }

    //-----------------------------------------------------------------------------------------
    virtual ~Vector()
    {}

    //-----------------------------------------------------------------------------------------
    void Assign(value_type v1, value_type v2)
    {
      assert(TDim==2);
      m_aData[0] = v1;
      m_aData[1] = v2;
    }

    //-----------------------------------------------------------------------------------------
    /** \brief Assign a value to all elements of the vector. */
    Vector& operator=(const value_type &val)
    {
      for (int i=0; i<TDim; ++i)
        m_aData[i] = val;
  
      return *this;
    }

    //-----------------------------------------------------------------------------------------
		Vector& operator+=(const Vector &v)
    {
      for (int i=0; i<TDim; ++i)
        m_aData[i] += v.m_aData[i];

      return *this;
    }

    //-----------------------------------------------------------------------------------------
		Vector& operator-=(const Vector &v)
    {
      for (int i=0; i<TDim; ++i)
        m_aData[i] -= v.m_aData[i];

      return *this;
    }

    //-----------------------------------------------------------------------------------------
	  Vector& operator*=(const double &v)
    {
      for (int i=0; i<TDim; ++i)
        m_aData[i] *= v;

      return *this;
    }

    //-----------------------------------------------------------------------------------------
	  Vector& operator/=(const double &v)
    {
      for (int i=0; i<TDim; ++i)
        m_aData[i] /= v;

      return *this;
    }

    //-----------------------------------------------------------------------------------------
    const value_type& operator[](std::size_t i) const
    {
      assert(i<TDim);
      return m_aData[i];
    }

    //-----------------------------------------------------------------------------------------
	  value_type& operator[](std::size_t i)
    {
      assert(i<TDim);
      return m_aData[i];
    }

  private:
    value_type m_aData[TDim];  ///< The vector data
  };

  //-------------------------------------------------------------------------------------------
  /** \brief Type for a three dimensional vector. */
  typedef Vector<double, 3> vec3d_type;

  //-------------------------------------------------------------------------------------------
  /** \brief Type for a two dimensional vector. */
  typedef Vector<double, 2> vec2d_type;

  //-------------------------------------------------------------------------------------------
  /** \brief Type for a two dimensional vector of integer numbers. */
  typedef Vector<int, 2> ivec2d_type;

  //-------------------------------------------------------------------------------------------
  /** \brief Calculate length of a vector. */
  template<typename TValType, int TDim>
  inline TValType abs(const Vector<TValType, TDim> &v)
  {
    TValType buf(0);
    for (int i=0; i<TDim; ++i)
      buf += mu::sqr(v[i]);

    return sqrt(buf);
  }

  //-------------------------------------------------------------------------------------------
  template<typename TValType, int TDim>
  inline Vector<TValType, TDim> operator+( const Vector<TValType, TDim> &v1, 
                                           const Vector<TValType, TDim> &v2 )
  {
    Vector<TValType, TDim> res;
    for (int i=0; i<TDim; ++i)
      res.m_aData[i] = v1.m_aData[i] + v2.m_aData[i];

    return res;
  }

  //-------------------------------------------------------------------------------------------
  template<typename TValType, int TDim>
  inline Vector<TValType, TDim> operator-( const Vector<TValType, TDim> &v1, 
                                           const Vector<TValType, TDim> &v2 )
  {
    Vector<TValType, TDim> res;
    for (int i=0; i<TDim; ++i)
      res.m_aData[i] = v1.m_aData[i] - v2.m_aData[i];

    return res;
  }

  //-------------------------------------------------------------------------------------------
  template<typename TValType, int TDim>
  inline Vector<TValType, TDim> operator*( const Vector<TValType, TDim> &v1, 
                                           const Vector<TValType, TDim> &v2 )
  {
    Vector<TValType, TDim> res;
    for (int i=0; i<TDim; ++i)
      res.m_aData[i] = v1.m_aData[i] * v2.m_aData[i];

    return res;
  }

  //-------------------------------------------------------------------------------------------
  template<typename TValType, int TDim>
  inline Vector<TValType, TDim> operator/( const Vector<TValType, TDim> &v1, 
                                           const Vector<TValType, TDim> &v2 )
  {
    Vector<TValType, TDim> res;
    for (int i=0; i<TDim; ++i)
      res.m_aData[i] = v1.m_aData[i] / v2.m_aData[i];

    return res;
  }

  //-------------------------------------------------------------------------------------------
  /** \brief Multiply every entry in a vector with a value. */
  template<typename TValType, int TDim>
  inline Vector<TValType, TDim> operator*( const TValType &val, 
                                           const Vector<TValType, TDim> &v2 )
  {
    Vector<TValType, TDim> res;
    for (int i=0; i<TDim; ++i)
      res.m_aData[i] = val * v2.m_aData[i];

    return res;
  }

  //-------------------------------------------------------------------------------------------
  /** \brief Multiply every entry in a vector with a value. */
  template<typename TValType, int TDim>
  inline Vector<TValType, TDim> operator*( const Vector<TValType, TDim> &v2,
                                           const TValType &val )
  {
    Vector<TValType, TDim> res;
    for (int i=0; i<TDim; ++i)
      res.m_aData[i] = val * v2.m_aData[i];

    return res;
  }
} // end of namespace mu


#endif // include guard