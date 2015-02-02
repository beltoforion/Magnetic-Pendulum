//---------------------------------------------------------------------------
//
//  Application utilities - circular buffer
//
//  File:          StatisticBuffer.h
//  Last changed:  21.05.2005
//  Author:        Ingo Berg
//
//---------------------------------------------------------------------------

#ifndef STAT_BUFFER_H
#define STAT_BUFFER_H

#include "auCircularBuffer.h"


namespace au
{

//---------------------------------------------------------------------------
//
//  Deklaration:  StatisticBuffer
//
//  Zweck:        Stellt eine Kontainerklasse dar, über deren Inhalt
//                statistische Größen, wie Mittelwert und Varianz
//                berechnet werden können.
//                Da sie auf einem Ringpuffer basiert ist sie ideal für
//                Berechnungen über ein bewegtes Fenster.
//
//---------------------------------------------------------------------------

template<typename TData>
class StatBuffer : private CircularBuffer<TData>
{
private:
  typedef StatBuffer<TData> self_type;
  typedef CircularBuffer<TData> parent_type;

public:
  using parent_type::Size;
  using parent_type::operator[];
  using parent_type::ElementAt;

  //--- Konstruktor / Destruktor -------------------------------------
  StatBuffer(unsigned size=0);
  StatBuffer(const self_type &buf);
  virtual ~StatBuffer();

  virtual void Reset(unsigned a_iSize = 0, const TData &val = TData());

  //--- Größe des Puffers ändern -------------------------------------
  virtual void Resize(unsigned size);
  virtual void Clear();

  //--- Hinzufügen von Werten (vorne im Puffer) ----------------------
  virtual void  Push(const TData& val);

  //--- Entfernen (letzten Wert) -------------------------------------
  virtual TData  Pop();

  //--- Abfragen der Statistikdaten ----------------------------------
  const TData& MeanValue();
  const TData& Variance();
  const TData& Min() const;
  const TData& Max() const;

private:
  TData  m_meanValue;  // Mittelwert über den Ringpuffer
  TData  m_sum;
  TData  m_sumOfSquares;
  TData  m_variance;
  mutable TData  m_min;
  mutable TData  m_max;

  mutable bool  m_bUpdateMax;
  mutable bool  m_bUpdateMin;

  void  ReCalculate();
};

//---------------------------------------------------------------------------
//
//  Implementation:  StatisticBuffer
//
//---------------------------------------------------------------------------

template<typename TData>
StatBuffer<TData>::StatBuffer(unsigned size)
  :CircularBuffer<TData>(size)
  ,m_meanValue()
  ,m_sum()
  ,m_sumOfSquares()
  ,m_variance()
  ,m_min()
  ,m_max()
  ,m_bUpdateMax(false)
  ,m_bUpdateMin(false)
{}

//---------------------------------------------------------------------------
template<typename TData>
StatBuffer<TData>::StatBuffer(const self_type &obj)
  :CircularBuffer<TData>(obj)
  ,m_meanValue(obj.m_meanValue)
  ,m_sum(obj.m_sum)
  ,m_sumOfSquares(obj.m_sumOfSquares)
  ,m_variance(obj.m_variance)
  ,m_min()
  ,m_max()
  ,m_MinBuf(obj.m_MinBuf)
  ,m_MaxBuf(obj.m_MaxBuf)
  ,m_bUpdateMax(obj.m_bUpdate)
  ,m_bUpdateMin(obj.m_bUpdate)
{}

//---------------------------------------------------------------------------
template<typename TData>
StatBuffer<TData>::~StatBuffer()
{}

//---------------------------------------------------------------------------
//  Hinzufügen / Entfernen von Werten zum Puffer
//---------------------------------------------------------------------------

template<typename TData>
void StatBuffer<TData>::Push(const TData& val)
{
  TData oldVal(Back());  // oldVal: Wert, der aus dem Puffer fallen wird.
  parent_type::Push(val);

  // Beitrag des neuen Wertes zu den Statistikdaten
  m_sum += val;
  m_sumOfSquares += val*val;

  // Beitrag des überschriebenen zu den Statistikdaten
  m_sum -= oldVal;
  m_sumOfSquares -= oldVal*oldVal;

  m_bUpdateMin = true;
  m_bUpdateMax = true;
}

//---------------------------------------------------------------------------
template<typename TData>
TData StatBuffer<TData>::Pop()
{
  TData val = parent_type::Pop();

  m_bUpdateMin = (val==m_min);
  m_bUpdateMax = (val==m_max);

  m_sum -= val;
  m_sumOfSquares -= val*val;

  return val;
}

//---------------------------------------------------------------------------
//  Managen des Puffers
//---------------------------------------------------------------------------

/** \brief Reset the buffer */
template<typename TData>
void StatBuffer<TData>::Reset(unsigned a_iSize, const TData &val)
{
  parent_type::Reset(a_iSize, val);
  ReCalculate();
}

template<typename TData>
void  StatBuffer<TData>::Resize(unsigned int newSize)
{
  parent_type::Resize(newSize);
  ReCalculate();
}

//---------------------------------------------------------------------------
template<typename TData>
void  StatBuffer<TData>::Clear()
{
  parent_type::Clear();

  m_meanValue = 0;
  m_variance = 0;
  m_sum = 0;
  m_sumOfSquares = 0;
  m_min = 0;
  m_max = 0;

  m_bUpdateMin = true;
  m_bUpdateMax = true;
}

//---------------------------------------------------------------------------
//  Neuberechnen der Statistikdaten
//---------------------------------------------------------------------------

template<typename TData>
void  StatBuffer<TData>::ReCalculate()
{
  if (!Size())
    throw InsufficientBufferSize();

  m_sum = TData();
  m_sumOfSquares = TData();

  m_max = m_min = this->operator[](0);
  for (unsigned i=0, size = Size(); i<size; ++i)
  {
    TData val(this->operator[](i));
    m_sum += val;
    m_sumOfSquares += val*val;

    m_min = std::min(m_min, val);
    m_max = std::max(m_max, val);
  }

  m_bUpdateMin = false;
  m_bUpdateMax = false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

template<typename TData>
const TData&  StatBuffer<TData>::MeanValue()
{
  unsigned s = Size();

  if (!s) throw InsufficientBufferSize();

  m_meanValue = m_sum / s;
  return m_meanValue;
}

//---------------------------------------------------------------------------
template<typename TData>
const TData&  StatBuffer<TData>::Variance()
{
  unsigned int s = Size();

  if (s<2)
    throw InsufficientBufferSize();

  m_variance = (m_sumOfSquares - (m_sum*m_sum / s)) / (s-1);
  return m_variance;
}

//---------------------------------------------------------------------------
template<typename TData>
const TData& StatBuffer<TData>::Min() const
{
  if (m_bUpdateMin)
  {
    m_min = *std::min_element(m_Data.begin(), m_Data.end());
    m_bUpdateMin = false;
  }

  return m_min;
}

//---------------------------------------------------------------------------
template<typename TData>
const TData& StatBuffer<TData>::Max() const
{
  if (m_bUpdateMax)
  {
    m_max = *std::max_element(m_Data.begin(), m_Data.end());
    m_bUpdateMax = false;
  }

  return m_max;
}

}  // end of namespace

#endif