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

    IniFile(const std::string &sFile = std::string());
   ~IniFile();
    IniFile(const IniFile &ref);

    const std::string& GetAsString(const std::string &sSection, const std::string &sKey) const;
    int GetAsInt(const std::string &sSection, const std::string &sKey) const;
    int GetAsInt(const std::string &sSection, const std::string &sKey, int nDefault) const;
    double GetAsFloat( const std::string &sSection, 
                       const std::string &sKey ) const;
    double GetAsFloat( const std::string &sSection, 
                       const std::string &sKey, 
                       double fDefault ) const;
#ifdef INI_FILE_PARSE_EXPR
    double GetAsFloatFromExpr( const std::string &sSection, 
                               const std::string &sKey ) const;
    double GetAsFloatFromExpr( const std::string &sSection, 
                               const std::string &sKey,
                               double fDefault) const;
#endif

    void Load(const std::string &sFile, EFlag eFlag = eNONE);
    void Reset();
    bool HasSection(const std::string &sSection) const;
    bool HasKey(const std::string &sSection, const std::string &sKey) const;
    bool HasKey(const std::string &sSection, const char *szKey) const;

  private:
    typedef std::map<std::string, std::string> section_type;
    typedef std::map<std::string, section_type> memfile_type;
    memfile_type m_MemFile;
  };
}  // end of namespace

#endif // include guard