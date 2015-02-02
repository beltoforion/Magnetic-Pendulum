#ifndef MU_BLOCK_MATRIX_H
#define MU_BLOCK_MATRIX_H

//--- Standard includes ---------------------------------------------------------------------
#include <memory.h>
#include <cstddef>  // for std::size_t
#include <cassert>
#include <string>
#include <fstream>
#include <stdexcept>


namespace mu
{
  //-----------------------------------------------------------------------------------------
  template<typename TData, typename TString = std::string>
  class BlockMatrix
  {
  private:
    typedef BlockMatrix<TData> self_type;

  public:
    typedef TData value_type;
    typedef TString string_type;

    //---------------------------------------------------------------------------------------
    BlockMatrix()
      :m_pData(NULL)
      ,m_rows(0)
      ,m_cols(0)
      ,m_size(0)
    {}

    //---------------------------------------------------------------------------------------
   ~BlockMatrix()
    {
      delete [] m_pData;
    }

    //---------------------------------------------------------------------------------------
    void Resize(std::size_t rows, std::size_t cols)
    {
      delete [] m_pData;
      m_pData = new value_type[rows*cols];
      Nullify();

      m_cols = cols;
      m_rows = rows;
      m_size = cols*rows;
    }

    //---------------------------------------------------------------------------------------
    self_type& operator=(const value_type &val)
    {
      for (std::size_t i=0; i<m_size; ++i)
        m_pData[i] = val;

      return *this;
    }

    //---------------------------------------------------------------------------------------
    const value_type* operator[](std::size_t idx_row) const
    {
      assert( m_pData );
      assert( (idx_row*m_cols)<m_size);
      return &m_pData[idx_row*m_cols];
    }

    //---------------------------------------------------------------------------------------
    value_type* operator[](std::size_t idx_row)
    {
      assert( m_pData );
      assert( (idx_row*m_cols)<m_size );
      return &m_pData[idx_row*m_cols];
    }

    //---------------------------------------------------------------------------------------
    std::size_t SizeRow() const
    {
      return m_rows;
    }

    //---------------------------------------------------------------------------------------
    std::size_t SizeCol() const
    {
      return m_cols;
    }

    //---------------------------------------------------------------------------------------
    std::size_t Size() const
    {
      return m_size;
    }

    //---------------------------------------------------------------------------------------
    void Nullify()
    {
      if (m_pData)
        std::memset( m_pData, 0, m_size*sizeof(value_type) );
    }

    //---------------------------------------------------------------------------------------
    const value_type* GetRawData() const
    {
      return m_pData;
    }

    //---------------------------------------------------------------------------------------
    void Write(const string_type &sFile) const
    {
      std::fstream file;
      file.open(sFile.c_str(), std::ios::out | std::ios::binary); 
      if (!file)
        throw std::runtime_error("Cant open file for matrix binary output");

      file.write( reinterpret_cast<const char*>(m_pData), 
                  static_cast<std::streamsize>(m_size*sizeof(value_type)) );
      file.close();
    }

    //---------------------------------------------------------------------------------------
    void Read(const string_type &sFile)
    {
      if (!m_pData)
        throw std::runtime_error("Reading to empty buffer");

      std::fstream file;
      file.open(sFile.c_str(), std::ios::in | std::ios::binary); 
      if (!file)
        throw std::runtime_error("Cant open file for matrix binary output");
  
      file.read( reinterpret_cast<char*>(m_pData), 
                  static_cast<std::streamsize>(m_size*sizeof(value_type)) );
      file.close();
    }

    value_type Max() const
    {
      TData *pMax = std::max_element(m_pData, m_pData + m_size);
      return *pMax;
    }

  private:
    value_type *m_pData;
    std::size_t m_rows;
    std::size_t m_cols;
    std::size_t m_size;
  };


} // end of namespace mu


#endif // include guard