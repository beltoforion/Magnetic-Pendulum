#ifndef AU_INI_FILE_H
#define AU_INI_FILE_H

//--- Standard includes ---------------------------------------------------------
#include <map>
#include <vector>
#include <string>


namespace au
{
  //-------------------------------------------------------------------------------
  class IniFile
  {
  public:
    enum EFlag
    {
      eNONE = 0,
      eIGNORE_CASE
    };

    IniFile(const std::wstring &sFile = std::wstring());
   ~IniFile();
    IniFile(const IniFile &ref);

    const std::wstring& GetAsString(const std::wstring &sSection, const std::wstring &sKey) const;
    int GetAsInt(const std::wstring &sSection, const std::wstring &sKey) const;
    int GetAsInt(const std::wstring &sSection, const std::wstring &sKey, int nDefault) const;
    double GetAsFloat( const std::wstring &sSection, 
                       const std::wstring &sKey ) const;
    double GetAsFloat( const std::wstring &sSection, 
                       const std::wstring &sKey, 
                       double fDefault ) const;

#ifdef INI_FILE_PARSE_EXPR
    double GetAsFloatFromExpr(const std::wstring &sSection, 
                              const std::wstring &sKey ) const;
    double GetAsFloatFromExpr(const std::wstring &sSection, 
                              const std::wstring &sKey,
                              double fDefault) const;
#endif

    void Load(const std::wstring &sFile, EFlag eFlag = eNONE);
    void Reset();
    bool HasSection(const std::wstring &sSection) const;
    bool HasKey(const std::wstring &sSection, const std::wstring &sKey) const;
    bool HasKey(const std::wstring &sSection, const wchar_t *szKey) const;

  private:
    typedef std::map<std::wstring, std::wstring> section_type;
    typedef std::map<std::wstring, section_type> memfile_type;
    memfile_type m_MemFile;
  };
}  // end of namespace

#endif // include guard