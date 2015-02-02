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
#ifndef AU_MAP_OF_POINTERS
#define AU_MAP_OF_POINTERS

#include <map>
#include <memory>


/** \brief Application utils namespace. */
namespace au
{
  /** \brief Implementation of mop, a (M)ap (o)f (P)ointers.
  
     Implemented in terms of a std::map.

     This class will manage the objects stored in its map implementation.
     Only pointers may be used in a mop object. The class takes ownership
     of the pointers and will take care of their destruction.

     Usage of a au::mop is similar to a std::map, the only difference is, that 
     the map does not own its objects when used with pointers.

     Do not delete pointers returned from any mop member function!
     Objects stored here may not throw an exception during their destruction!
     Objects stored here must be have a clon member function!
  */
  template< typename TKey, 
            typename TMap, 
            typename TSorter = std::less<TKey> >
  class mop
  {
  private:
    typedef TMap* mapped_type;
    typedef TKey key_type;
    typedef std::map<key_type, mapped_type, TSorter> map_type;

  public:
    typedef typename map_type::const_iterator const_iterator;
    typedef typename map_type::reverse_iterator reverse_iterator;
    typedef typename map_type::iterator iterator;

    //------------------------------------------------------------------------------
    mop()
      :m_Map()
    {
    }

    //------------------------------------------------------------------------------
    /** \brief Copy Constructor.
    
      Deep copy based on the clone function of the stored objects.
    */
    mop(const mop &a_Obj)
      :m_Map()
    {
      const_iterator item = a_Obj.m_Map.begin();
      for (; item!=m_Map.end(); ++item)
        m_Map[item->first] = static_cast<mapped_type>(item->second->Clone());
    }

    //------------------------------------------------------------------------------
    virtual ~mop()
    {
      clear();
    }

    //------------------------------------------------------------------------------
    typename mapped_type& operator[](const key_type &a_Key)
    {
      map_type::const_iterator item = find(a_Key);
      if (item!=m_Map.end())
        delete item->second;

      return m_Map[a_Key];
    }

    //------------------------------------------------------------------------------
    typename const_iterator find(const TKey &a_Key) const
    {
      return m_Map.find(a_Key);
    }

    //------------------------------------------------------------------------------
    typename iterator find(const TKey &a_Key) 
    {
      return m_Map.find(a_Key);
    }

    //------------------------------------------------------------------------------
    typename reverse_iterator rbegin()
    {
      return m_Map.rbegin();
    }

    //------------------------------------------------------------------------------
    typename reverse_iterator rend()
    {
      return m_Map.rend();
    }

    //------------------------------------------------------------------------------
    typename const_iterator begin()
    {
      return m_Map.begin();
    }

    //------------------------------------------------------------------------------
    typename const_iterator end()
    {
      return m_Map.end();
    }

    //------------------------------------------------------------------------------
    void clear()
    {
      typename map_type::const_iterator item = m_Map.begin();
      for (; item!=m_Map.end(); ++item)
        delete item->second;

      m_Map.clear();
    }

    //------------------------------------------------------------------------------
    std::size_t size() const
    {
      return m_Map.size();
    }

  private:
    map_type  m_Map;
  };
}

#endif

