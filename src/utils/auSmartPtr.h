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
#ifndef AU_SMART_PTR_H
#define AU_SMART_PTR_H

#include <cassert>


namespace au
{
  struct refcounter
  {
    refcounter(void *ptr) : m_count(0), m_ptr(ptr) {}
    mutable long m_count;
    void *m_ptr;
  };

  //------------------------------------------------------------------------------
  template<typename T>
  class smart_ptr
  {
  public:
      typedef T element_type;
      typedef T* ptr_type;

      //--------------------------------------------------------------------------
      smart_ptr()
        :m_refcount(NULL)
      {
      }

      //--------------------------------------------------------------------------
      explicit smart_ptr(ptr_type p)
        :m_refcount(NULL)
      {
        if (p)
        {
          m_refcount = new refcounter(p);
          ++m_refcount->m_count;
        }
      }
      
      //--------------------------------------------------------------------------
      smart_ptr(const smart_ptr &p)
        :m_refcount(0)
      {
        if (p.m_refcount)
        {
          m_refcount = p.m_refcount;
          ++m_refcount->m_count;
        }
      }
      
      //--------------------------------------------------------------------------
      template<typename T2>
      smart_ptr(const smart_ptr<T2> &p)
        :m_refcount(0)
      {
        if (p.m_refcount)
        {
          assert(dynamic_cast<ptr_type>((typename smart_ptr<T2>::ptr_type)p.m_refcount->m_ptr));
          m_refcount = p.m_refcount;
          ++m_refcount->m_count;
        }
      }

      //--------------------------------------------------------------------------
     ~smart_ptr()
      {
        if (m_refcount && --m_refcount->m_count==0)
        {
          delete (ptr_type)m_refcount->m_ptr;
          delete m_refcount;
        }
      };

      //--------------------------------------------------------------------------
      ptr_type operator->()
      {
        return (m_refcount) ? (ptr_type)(m_refcount->m_ptr) : NULL;
      }

      //--------------------------------------------------------------------------
      const ptr_type operator->() const
      {
        return (m_refcount) ? (ptr_type)(m_refcount->m_ptr) : NULL;
      }

      //--------------------------------------------------------------------------
      ptr_type get()
      {
        return (m_refcount) ? (ptr_type)(m_refcount->m_ptr) : NULL;
      }

      //--------------------------------------------------------------------------
      const ptr_type get() const
      {
        return (m_refcount) ? (ptr_type)(m_refcount->m_ptr) : NULL;
      }
      
      //--------------------------------------------------------------------------
      smart_ptr& operator=(const smart_ptr &p)
      {
        if (this==&p)
          return *this;

        if (p.m_refcount)
          ++p.m_refcount->m_count;

        if (m_refcount && --m_refcount->m_count==0)
          delete m_refcount;

        m_refcount = p.m_refcount;
        return *this;
      }

      //--------------------------------------------------------------------------
      //template<typename T2>
      //smart_ptr& operator=(const smart_ptr<T2> &p)
      //{
      //  if (this==&p)
      //    return *this;

      //  if (p.m_refcount)
      //    ++p.m_refcount->m_count;

      //  if (m_refcount && --m_refcount->m_count==0)
      //    delete m_refcount;

      //  m_refcount = p.m_refcount;
      //  return *this;
      //}

      //--------------------------------------------------------------------------
      long use_count() const
      {
        return (m_refcount) ? m_refcount->m_count : 0;
      }

  public:
      mutable refcounter *m_refcount;
  };
}

#endif
