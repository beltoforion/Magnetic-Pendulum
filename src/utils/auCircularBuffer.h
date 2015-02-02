//---------------------------------------------------------------------------
//
//  Application utilities - circular buffer
//
//  File:          CircularBuffer.h
//  Last changed:  21.05.2005
//  Author:        Ingo Berg
//
//---------------------------------------------------------------------------

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

//--- C includes ------------------------------------------------------------
#include <cassert>

//--- STD includes ----------------------------------------------------------
#include <algorithm>
#include <stdexcept>
#include <exception>
#include <numeric>
#include <vector>

#if defined(_MSC_VER)
  #undef min
  #undef max
#endif


namespace au // Application utilities
{

//---------------------------------------------------------------------------
/// \brief  Error class that is thrown in the case of trying to access the buffer
/// beyond its min/max limits.
struct InsufficientBufferSize : std::runtime_error
{
  InsufficientBufferSize(std::string msg="Circular buffer has zero size!")
    :std::runtime_error(msg)
  {}
};

//---------------------------------------------------------------------------
/** \brief A container class that is implemented as a circular buffer.
    A circular buffer can be imagined as a fixed size stack.
*/
template<typename TData>
class CircularBuffer 
{
private:
  typedef CircularBuffer<TData> self_type;

public:

  //---  Constructor / Copy-constructor / Destructor -----------------
          CircularBuffer(unsigned a_size = 0);
          CircularBuffer(const CircularBuffer &buf);
  virtual ~CircularBuffer();

  /** \brief Reset the buffer */
  virtual void Reset(unsigned a_iSize = 0, const TData &val = TData());

  //--- Hinzufügen von Werten ----------------------------------------
  virtual void  Push(const TData& val);
  void  Preset(const TData &val);

  //--- Entfernen (letzten Wert) -------------------------------------
  virtual TData  Pop();

  //--- Größe des Puffers ändern -------------------------------------
  virtual void Resize(unsigned int size);
  virtual void Clear();
  unsigned Size() const;
  unsigned Count() const;

  //--- Zugriff auf Werte --------------------------------------------
  TData&  operator[](unsigned);
  const TData&  operator[](unsigned) const;

  TData&  ElementAt(unsigned);
  const TData&  ElementAt(unsigned) const;

  TData&  RawOrderElementAt(unsigned);
  const TData&  RawOrderElementAt(unsigned) const;

  TData&  Front();
  TData&  Back();

protected:
  std::vector<TData> m_Data; ///< The data buffer

private:
  unsigned m_zeroPos;        ///< The "zero position" index in the buffer.

  /** \brief Number of elements that are currently in the buffer.
             Only elements that were explicitely added to the
             buffer are counted. 
             m_count is either smaller or equal to the buffer
             size. (equal once you calles push at least size
             times)
  */
  unsigned m_count; 

  //--- Konvertieren eines Indexwertes -------------------------------
  inline unsigned int IndexToCircularIndex(unsigned int idx) const;

};


//---------------------------------------------------------------------------
//
//
//  Implementation:  CircularBuffer
//
//
//---------------------------------------------------------------------------

/// \brief Constructor
///
/// Create a circular buffer of size a_size and initialises it's content with default
/// constructed values of type TData.
template<typename TData>
CircularBuffer<TData>::CircularBuffer(unsigned a_iSize)
  :m_Data(a_iSize, TData())
  ,m_zeroPos(0)
  ,m_count(0)
{
}

//---------------------------------------------------------------------------
/// \brief Copy constructor
template<typename TData>
CircularBuffer<TData>::CircularBuffer(const self_type &val)
  :m_Data(val.m_Data)
  ,m_zeroPos(val.m_zeroPos)
  ,m_count(val.m_count)
{
}

//---------------------------------------------------------------------------
/// \brief Destructor
template<typename TData>
CircularBuffer<TData>::~CircularBuffer()
{
}

//---------------------------------------------------------------------------
template<typename TData>
void CircularBuffer<TData>::Reset(unsigned a_iSize, const TData &val)
{
  Resize(a_iSize);
  Preset(val);
}

//---------------------------------------------------------------------------
/// \brief Add an element to the buffer.
template<typename TData>
void  CircularBuffer<TData>::Push(const TData& val)
{
  if (m_Data.empty())
    throw InsufficientBufferSize();

  m_Data[m_zeroPos++] = val;

  unsigned sz = m_Data.size();
  if (sz)
    m_zeroPos %= sz;
  else
    m_zeroPos = 0;

  m_count = std::min(++m_count, sz);
}

//---------------------------------------------------------------------------
/// \brief Fill the buffer with a certain value.
template<typename TData>
void  CircularBuffer<TData>::Preset(const TData &val)
{
  unsigned int i, size = Size();
  for (i=0; i<size; i++)
    m_Data[i] = val;

  m_count = size;
}

//---------------------------------------------------------------------------
/// \brief Remove the element that was last inserted (the upermost one) from the buffer.
///
/// The element that was inserted last will be returned and removed from the buffer.
/// All other elements "move" one position upwards. At the "end" of the buffer
/// a default constructed element will be inserted.<br>
/// The "end" and "start" of the buffer are virtual position markers.
/// due to the fact that a circular buffer has no real "start" or "end". They denote
/// the oldest and youngest elements in the buffer.
/// \return The element that was removed from the buffer.
/// \exception EInsufficientBufferSize if the buffer has zero size.
template<typename TData>
TData CircularBuffer<TData>::Pop()
{
  if (m_Data.empty())
    throw InsufficientBufferSize();

  TData rv = m_Data[0]; // Element herausholen
  m_Data[0] = TData();  // im Speicher löschen d.h. durch
                         // Default-konstruiertes ersetzen

  if (m_zeroPos==0)
  {
    m_zeroPos = m_Data.size()-1;
  }
  else
  {
    --m_zeroPos;
  }

  if (m_count) --m_count;

  return rv;
}

//---------------------------------------------------------------------------
/// \brief Changes the size of the buffer.
///
/// The order of the element will remain.
/// In case of a shrinkage of the buffer the elements that were inserted last will be
/// cut. In case of buffer extension new default constructed elements will be inserted at
/// the buffer end.
template<typename TData>
void CircularBuffer<TData>::Resize(unsigned size)
{
  self_type buf(*this);

  unsigned oldsize = buf.m_Data.size();  // alte Größe speichern
  m_Data.resize(size);                   // Größe ändern

  m_zeroPos = 0;

  for (unsigned int i = 0; i<size; i++)
  {
    if (i<oldsize)
    {
      (*this)[i] = buf[i];
    }
    else
    {
      (*this)[i] = TData();
    }
  }
}

//---------------------------------------------------------------------------
template<typename TData>
unsigned CircularBuffer<TData>::Size() const
{
  return (unsigned)m_Data.size();
}

//---------------------------------------------------------------------------
template<typename TData>
unsigned int CircularBuffer<TData>::Count() const
{
  return m_count;
}

//---------------------------------------------------------------------------
template<typename TData>
void CircularBuffer<TData>::Clear()
{
  m_Data.assign(m_Data.size(), TData());
  m_zeroPos = 0;
  m_count = 0;
}

//---------------------------------------------------------------------------
//  Elementzugriff
//---------------------------------------------------------------------------

template<typename TData>
TData& CircularBuffer<TData>::operator[](unsigned int idx)
{
  unsigned int cidx = IndexToCircularIndex(idx);
  return m_Data[cidx];
}


//---------------------------------------------------------------------------
template<typename TData>
const TData& CircularBuffer<TData>::operator[](unsigned int idx) const
{
  // Zweck:  const überladene Version des Zugriffsoperators ohne Bereichsprüfung

  unsigned int cidx = IndexToCircularIndex(idx);
  return m_Data[cidx];
}


//---------------------------------------------------------------------------
template<typename TData>
TData&  CircularBuffer<TData>::ElementAt(unsigned int idx)
{
  // Zweck:  Zugriffsoperator mit Bereichsprüfung, Elemente
  //         werden sortiert gemäß Eingang zurückgegeben.

  unsigned int cidx = IndexToCircularIndex(idx);
  return m_Data.at(cidx);
}


template<typename TData>
const TData&  CircularBuffer<TData>::ElementAt(unsigned int idx) const
{
  // Zweck:  const überladene Version des Zugriffsoperators ohne Bereichsprüfung

  unsigned int cidx = IndexToCircularIndex(idx);
  return m_Data.at(cidx);
}


template<typename TData>
TData&  CircularBuffer<TData>::RawOrderElementAt(unsigned int idx)
{
  // Zweck:  Zugriff auf die Daten in der Reihenfolge der Rohdaten

  return m_Data.at(idx);
}


template<typename TData>
const TData&  CircularBuffer<TData>::RawOrderElementAt(unsigned int idx) const
{
  // Zweck:  Zugriff auf die Daten in der Reihenfolge der Rohdaten
  //         const Überlagerte Version

  return m_Data.at(idx);
}

//--- erstes Element --------------------------------------------------------
template<typename TData>
TData& CircularBuffer<TData>::Front()
{
  if (m_Data.empty())
    throw InsufficientBufferSize();

  return m_Data[0];
}

//--- letztes Element -------------------------------------------------------
template<typename TData>
TData&  CircularBuffer<TData>::Back()
{
  if (m_Data.empty())
    throw InsufficientBufferSize();

  return m_Data[m_Data.size()-1];
}

//---------------------------------------------------------------------------
//  Hilfsfunktionen
//---------------------------------------------------------------------------

// Zweck:       Umrechnen des Indexwertes in einen Zirkularindex.
// Erläuterung: Im Ringpuffer gibt es keine sinnvolle Ordnung, weil er sich
//              ständig selbst überschreibt. Als einzig sinnvoller Ersatz
//              dient die Reihenfolge in der die Elemente abgespeichert
//              wurden.
// Beiepiel:    Index 5 muß umgerechnet werden in den Index des Elementes was
//              als 5. letzen gespeichert wurde
template<typename TData>
unsigned int CircularBuffer<TData>::IndexToCircularIndex(unsigned int idx) const
{

  int pos = m_zeroPos - 1 - idx;
  if (pos<0)
    pos = m_Data.size() + pos;

  return pos;
}


}

#endif
