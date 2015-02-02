/*
  Copyright (C) 2006 Ingo Berg

  Permission is hereby granted, free of charge, to any person obtaining a copy of this 
  software and associated documentation files (the "Software"), to deal in the Software
  without restriction, including without limitation the rights to use, copy, modify, 
  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
  permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or 
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/
#ifndef AU_VECTOR_OF_POINTERS
#define AU_VECTOR_OF_POINTERS

#include <vector>


/** \brief Application utils namespace. */
namespace au
{
  /** \brief Implementation of vop, a (V)ector (o)f (P)ointers.
  
     Implemented in terms of a std::vector.

     This class will manage the objects stored in its map implementation.
     Only pointers may be used in a vop object. The class takes ownership
     of the pointers and will take care of their destruction.

     Usage of a au::vop is similar to a std::map, the only difference is, that 
     the vector does not own its objects when used with pointers.

     Do not delete pointers returned from any mop member function!
     Objects stored here may not throw an exception during their destruction!
     Objects stored here must be have a clon member function!
  */
  template<typename TVal>
  class vop
  {
  private:
    typedef TVal value_type;
    typedef std::vector<value_type> vec_type;

    vec_type m_vec;

  public:
    typedef typename vec_type::const_iterator const_iterator;
    typedef typename vec_type::reverse_iterator reverse_iterator;
    typedef typename vec_type::iterator iterator;

    //------------------------------------------------------------------------------
    vop()
      :m_vec()
    {
    }

    //------------------------------------------------------------------------------
    vop(const mop &a_Obj)
      :m_vec()
    {
      assert(false);
    }

    //------------------------------------------------------------------------------
    virtual ~vop()
    {
      clear();
    }

    //------------------------------------------------------------------------------
    void push_back(const value_type& ptr)
    {
      m_vec.push_back(ptr);
    }

    //------------------------------------------------------------------------------
    void pop_back()    
    {
      if (!m_vec.empty())
      {
        delete m_vec.back();
        m_vec.pop_back();
      }          
    }

    //------------------------------------------------------------------------------
    const value_type& back() const
    {
      return m_vec.back();
    }

    //------------------------------------------------------------------------------
    value_type& back()
    {
      return m_vec.back();
    }

    //------------------------------------------------------------------------------
    const value_type& operator[](unsigned a_iIdx)
    {
      return m_vec[a_iIdx];
    }

    //------------------------------------------------------------------------------
    void clear()
    {
      for (unsigned i=0; i<m_vec.size(); ++i)
      {
        delete m_vec[i];
        m_vec[i]=0;
      }

      m_vec.clear();
    }

    //------------------------------------------------------------------------------
    std::size_t size() const
    {
      return m_vec.size();
    }

    //------------------------------------------------------------------------------
    bool empty() const
    {
      return m_vec.empty();
    }
  };
}

#endif

