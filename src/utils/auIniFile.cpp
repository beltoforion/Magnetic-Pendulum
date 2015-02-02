#include "auIniFile.h"

//--- Standard includes -----------------------------------------------------
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <cstdio>

//--- Utility classes -------------------------------------------------------
#include "suStringTokens.h"
#include "suUtility.h"

#ifdef INI_FILE_PARSE_EXPR
#include "muParserDLL.h"
#endif


namespace au
{
  //---------------------------------------------------------------------------
  IniFile::IniFile(const std::string &sFile)
    :m_MemFile()
  {
    if (sFile.length())    
      Load(sFile);
  }

  //---------------------------------------------------------------------------
  IniFile::~IniFile()
  {}

  //---------------------------------------------------------------------------
  IniFile::IniFile(const IniFile &ref)
    :m_MemFile(ref.m_MemFile)
  {}

  //---------------------------------------------------------------------------
  const std::string &IniFile::GetAsString(const std::string &sSection, const std::string &sKey) const
  {
    memfile_type::const_iterator it(m_MemFile.find(sSection) );
    if (it==m_MemFile.end())
      throw std::runtime_error("Ini file section not found.");

    section_type::const_iterator itKey(it->second.find(sKey));
    if (itKey==it->second.end())
    {
      std::stringstream ss;
      ss << "Key " << sKey << " not found in section " << sSection << "\n";
      throw std::runtime_error(ss.str());
    }
  
    return itKey->second;
  }

  //---------------------------------------------------------------------------
  /** \brief Get INI file entry as integer.
      \throw std::runtime_error in case key or section does not exist
      \throw std::bad_cast in case conversion fails
  */
  int IniFile::GetAsInt(const std::string &sSection, const std::string &sKey) const
  {
    int nRet(0),
        nStat(sscanf(GetAsString(sSection, sKey).c_str(), "%d", &nRet));

    if (nStat!=1)
      throw std::bad_cast("Cant convert to integer");
    
    return nRet;
  }

  //---------------------------------------------------------------------------
  int IniFile::GetAsInt( const std::string &sSection, 
                         const std::string &sKey, 
                         int nDefault ) const
  try
  {
    return GetAsInt(sSection, sKey);
  }
  catch(std::runtime_error &)
  {
    return nDefault;
  }

  //---------------------------------------------------------------------------
  double IniFile::GetAsFloat( const std::string &sSection, 
                              const std::string &sKey ) const
  {
    double fRet(0);
    int nStat(sscanf(GetAsString(sSection, sKey).c_str(), "%lf", &fRet));
    
    if (nStat!=1)
      throw std::bad_cast("Cant convert to double");
    
    return fRet;
  }

  //---------------------------------------------------------------------------
  double IniFile::GetAsFloat( const std::string &sSection, 
                              const std::string &sKey, 
                              double fDefault ) const
  try
  {
    return GetAsFloat( sSection, sKey) ;
  }
  catch(...)
  {
    return fDefault;
  }

#ifdef INI_FILE_PARSE_EXPR
  //-----------------------------------------------------------------------------------------
  /** \brief Converts a string to a double value taking mathematical expressions 
             into account. 
  */
  double IniFile::GetAsFloatFromExpr( const std::string &sSection, 
                                      const std::string &sKey ) const
  try
  {
    std::string sExpr( GetAsString(sSection, sKey) );

    parser_handle hParser( mupInit() );

    mupSetExpr( hParser, sExpr.c_str() );  
    double fRet = mupEval(hParser);
  
    if (mupError())
      throw std::bad_cast( mupGetErrorMsg() );

    mupRelease(hParser);

    return fRet;
  }
  catch(std::bad_cast &exc)
  {
    std::stringstream ss;
    ss << "Can't convert \"[" << sSection.c_str() << "]/"  
       << sKey.c_str()
       << "\" to floating point falue.\n"
       << "(" << exc.what() << ")";

    throw std::runtime_error( ss.str() );
  }

  //-----------------------------------------------------------------------------------------
  double IniFile::GetAsFloatFromExpr( const std::string &sSection, 
                                      const std::string &sKey,
                                      double fDefault) const
  try
  {
    return GetAsFloatFromExpr(sSection, sKey);
  }
  catch(...)
  {
    return fDefault;
  }
#endif

  //---------------------------------------------------------------------------
  void IniFile::Reset()
  {
    m_MemFile.clear();
  }

  //---------------------------------------------------------------------------
  bool IniFile::HasSection(const std::string &sSection) const
  {
    memfile_type::const_iterator it(m_MemFile.find(sSection));
    return it!=m_MemFile.end();
  }

  //---------------------------------------------------------------------------
  bool IniFile::HasKey(const std::string &sSection, const char *szKey) const
  {
    std::string sKey(szKey);
    return HasKey(sSection, sKey);
  }

  //---------------------------------------------------------------------------
  bool IniFile::HasKey(const std::string &sSection, const std::string &sKey) const
  {
    memfile_type::const_iterator it( m_MemFile.find(sSection) );
    if (it==m_MemFile.end())
      return false;

    section_type::const_iterator itKey(it->second.find(sKey));
    if (itKey==it->second.end())
      return false;
    
    return true;
  }

  //---------------------------------------------------------------------------
  void IniFile::Load(const std::string &sFile, EFlag eFlag)
  {
    Reset();

    std::ifstream ifs(sFile.c_str());
    if (!ifs)
    {
      std::stringstream msg;
      msg << "Can't open configuration file \"" << sFile << "\" for reading.";
      throw std::runtime_error(msg.str());
    }

    std::string sLine, 
                sSection;

    while (!ifs.eof())
    {
      std::getline(ifs, sLine);
      if (!sLine.length() || sLine[0]==';')
        continue;

      // check for section tags
      const std::size_t start(sLine.find('[')),
                        end(sLine.find(']'));
      if (start!=std::string::npos && end!=std::string::npos)
      {
        sSection.assign( &sLine[start+1], 
                         &sLine[end] );

        if (eFlag==au::IniFile::eIGNORE_CASE)
          sSection = su::to_upper(sSection);

        continue; // we found a section identifier
      }

      if (sSection.length())
      {
        su::StringTokens<std::string> tok(sLine, "=");
        if (tok.Count()!=2)
          continue;

        std::string sKey( su::trim( su::to_upper(tok[0]) ) ), 
                    sVal( tok[1] );
        m_MemFile[sSection][sKey] = sVal;
      }
    } // while not eof
  }
};