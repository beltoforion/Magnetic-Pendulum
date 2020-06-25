#include "../stdafx.h"
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
#include "utWideExceptions.h"
#include "../muparser/muParser.h"


namespace au
{
    //---------------------------------------------------------------------------
    IniFile::IniFile(const std::wstring &sFile)
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
    const std::wstring &IniFile::GetAsString(const std::wstring &sSection, const std::wstring &sKey) const
    {
        memfile_type::const_iterator it(m_MemFile.find(sSection));
        if (it == m_MemFile.end())
        {
            throw utils::wruntime_error(_T("Ini file section not found."));
        }

        section_type::const_iterator itKey(it->second.find(sKey));
        if (itKey == it->second.end())
        {
            std::wstringstream ss;
            ss << _T("Key ") << sKey << _T(" not found in section ") << sSection << _T("\n");
            throw utils::wruntime_error(ss.str());
        }

        return itKey->second;
    }

    //---------------------------------------------------------------------------
    /** \brief Get INI file entry as integer.
        \throw std::runtime_error in case key or section does not exist
        \throw std::bad_cast in case conversion fails
        */
    int IniFile::GetAsInt(const std::wstring &sSection, const std::wstring &sKey) const
    {
        int nRet(0),
            nStat(swscanf(GetAsString(sSection, sKey).c_str(), _T("%d"), &nRet));

        if (nStat != 1)
        {
            throw utils::wruntime_error(_T("Cant convert to integer"));
        }

        return nRet;
    }

    //---------------------------------------------------------------------------
    int IniFile::GetAsInt(const std::wstring &sSection,
                          const std::wstring &sKey,
                          int nDefault) const
    try
    {
        return GetAsInt(sSection, sKey);
    }
    catch (std::runtime_error &)
    {
        return nDefault;
    }

    //---------------------------------------------------------------------------
    double IniFile::GetAsFloat(const std::wstring &sSection,
                               const std::wstring &sKey) const
    {
        double fRet(0);
        int nStat(swscanf(GetAsString(sSection, sKey).c_str(), _T("%lf"), &fRet));

        if (nStat != 1)
        {
            throw utils::wruntime_error(_T("Cant convert to double"));
        }

        return fRet;
    }

    //---------------------------------------------------------------------------
    double IniFile::GetAsFloat(const std::wstring &sSection,
        const std::wstring &sKey,
        double fDefault) const
    {
        try
        {
            return GetAsFloat(sSection, sKey);
        }
        catch (...)
        {
            return fDefault;
        }
    }

#ifdef INI_FILE_PARSE_EXPR
    //-----------------------------------------------------------------------------------------
    /** \brief Converts a string to a double value taking mathematical expressions
               into account.
    */
    double IniFile::GetAsFloatFromExpr(const std::wstring &sSection,
                                       const std::wstring &sKey) const
    try
    {
        std::wstring sExpr(GetAsString(sSection, sKey));

        auto parser = mu::Parser();

        parser.SetExpr(sExpr.c_str());
        double fRet = parser.Eval();

        return fRet;
    }
    catch (utils::wruntime_error &exc)
    {
        std::wstringstream ss;
        ss << _T("Can't convert \"[") << sSection.c_str() << _T("]/")
           << sKey.c_str()
           << _T("\") to floating point value.\n")
           << _T("(") << exc.what() << _T(")");

        throw utils::wruntime_error(ss.str());
    }

    //-----------------------------------------------------------------------------------------
    double IniFile::GetAsFloatFromExpr(const std::wstring &sSection,
                                       const std::wstring &sKey,
                                       double fDefault) const
    try
    {
        return GetAsFloatFromExpr(sSection, sKey);
    }
    catch (...)
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
    bool IniFile::HasSection(const std::wstring &sSection) const
    {
        memfile_type::const_iterator it(m_MemFile.find(sSection));
        return it != m_MemFile.end();
    }

    //---------------------------------------------------------------------------
    bool IniFile::HasKey(const std::wstring &sSection, const wchar_t *szKey) const
    {
        std::wstring sKey(szKey);
        return HasKey(sSection, sKey);
    }

    //---------------------------------------------------------------------------
    bool IniFile::HasKey(const std::wstring &sSection, const std::wstring &sKey) const
    {
        memfile_type::const_iterator it(m_MemFile.find(sSection));
        if (it == m_MemFile.end())
            return false;

        section_type::const_iterator itKey(it->second.find(sKey));
        if (itKey == it->second.end())
            return false;

        return true;
    }

    //---------------------------------------------------------------------------
    void IniFile::Load(const std::wstring &sFile, EFlag eFlag)
    {
        Reset();

        std::wifstream ifs(sFile.c_str());
        if (!ifs)
        {
            std::wstringstream msg;
            msg << _T("Can't open configuration file \"") << sFile << _T("\" for reading.");
            throw utils::wruntime_error(msg.str());
        }

        std::wstring sLine, sSection;

        while (!ifs.eof())
        {
            std::getline(ifs, sLine);
            if (!sLine.length() || sLine[0] == ';')
                continue;

            // check for section tags
            const std::size_t start(sLine.find('[')), end(sLine.find(']'));
            if (start != std::string::npos && end != std::string::npos)
            {
                sSection.assign(&sLine[start + 1],
                    &sLine[end]);

                if (eFlag == au::IniFile::eIGNORE_CASE)
                    sSection = su::to_upper(sSection);

                continue; // we found a section identifier
            }

            if (sSection.length())
            {
                su::StringTokens<std::wstring> tok(sLine, _T("="));
                if (tok.Count() != 2)
                    continue;

                std::wstring sKey(su::trim(su::to_upper(tok[0]))), sVal(tok[1]);
                m_MemFile[sSection][sKey] = sVal;
            }
        } // while not eof
    }
};