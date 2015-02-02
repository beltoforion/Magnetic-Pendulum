//---------------------------------------------------------------------------
//
//  String utilities - string tokenizer
//
//  File:          suStringTokens.h
//  Last changed:  30.05.2005
//  Author:        Ingo Berg
//
//
//  History:
//
//  30.05.2005
//        - Moved member function definitions into class body
//          to prevent internal compiler error on MSVC 7.1
//        - Removed all const char_type related member functions
//          string_type is now the only type of input
//        - rewritten to allow multiple delimiter characters
//        - ETokenExceotion renamed to TokenExceptions
//
//---------------------------------------------------------------------------

#ifndef SU_STRING_TOKENS_H
#define SU_STRING_TOKENS_H

#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>


namespace su
{

struct TokenNotFound : public std::runtime_error
{
  TokenNotFound() : std::runtime_error("Token not found") {};
};

//---------------------------------------------------------------------------
//
//  Deklaration:  StringTokens
//
//  Zweck:  Klasse zum Zerlegen eines durch beliebige Trennungszeichen
//          (Tokens) geteilten Strings. Die Teilstrings werden in einem
//          vector abgelegt und können über ihren Index, oder eine
//          Suchfunktion angesprochen werden.
//
//  Einschränkungen:  Die Klasse ist als Template geschrieben. Der Template
//                    Parameter entspricht dem Typ des Ausgangsstringes.
//
//---------------------------------------------------------------------------

template<typename TData>
/* final */ class StringTokens
{
public:
        typedef TData string_type;
        typedef typename TData::value_type char_type;

private:    
        typedef std::vector<string_type> storage_type;

public:

    //--- Konstruktor -----------------------------------------------------------
    StringTokens(const string_type &a_sInput, const string_type &a_sDelim = " ")
      :m_delimiter(a_sDelim)
      ,m_tokens()
    {
      Tokenize(a_sInput, a_sDelim);
    }

    //--- Default Konstruktor ---------------------------------------------------
    StringTokens()
      :m_delimiter(string_type(" "))
      ,m_tokens()
    {}

    //--- Destruktor ------------------------------------------------------------
   ~StringTokens()
    {}

    //---------------------------------------------------------------------------
    void Tokenize(const string_type &a_sInput, const string_type &a_sDelim = " ")
    {
      string_type buf(a_sInput);

      int  j = 0;
      for (int i=0; i<(int)buf.length(); ++i)
      {
        if ( a_sDelim.find(buf[i])!=buf.npos )
        {
          if (i!=j)
            m_tokens.push_back( TData(buf.substr(j, i-j).c_str()) );
          j = i+1;
        }
      }

      if ( j < (int)(buf.length()) )
        m_tokens.push_back( string_type(buf.substr(j).c_str()) );
    }

    //---------------------------------------------------------------------------
    typename const storage_type& Get() const
    {
      return m_tokens;
    }

    //---------------------------------------------------------------------------
    unsigned int Count() const
    {
      return static_cast<unsigned int>(m_tokens.size());
    }

    //---------------------------------------------------------------------------
    // Zweck:  Rückgabe des Tokens, das an idx'ter Stelle gefunden wurde.
    //         wird ein Index angegeben, der nicht der Tokenanzahl
    //         entspricht, so wird eine Exception ausgelöst.
    const TData& operator[](unsigned int idx) const
    {
      try
      {
        return m_tokens.at(idx);
      }
      catch(std::out_of_range&)
      {
        throw TokenNotFound();
      }
    }

    //---------------------------------------------------------------------------
    TData& operator[](unsigned idx)
    {
      try
      {
        return m_tokens.at(idx);
      }
      catch(std::out_of_range&)
      {
        throw TokenNotFound();
      }
    }

    //---------------------------------------------------------------------------
    int IndexOf(const TData &str) const
    {
      storage_type::const_iterator  item, 
                                    b = m_tokens.begin(), 
                                    e = m_tokens.end();
                                       
      item = std::find(b, e, str);
      return (item == e) ? -1 : item-b;
    }


private:
    StringTokens(const StringTokens &a_Tokens);
    StringTokens &operator=(const StringTokens &a_Tokens);

    string_type  m_delimiter;
    storage_type  m_tokens;
};

} // end of namespace

#endif
