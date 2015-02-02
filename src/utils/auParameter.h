//---------------------------------------------------------------------------
//
//  Windows utilities - command line parameter parser
//
//  File:          auParameter.h
//  Last changed:  28.05.2005
//  Author:        Ingo Berg
//
//  Modification history:
//    - 03.06.2005 namespace changed from au to wu
//
//---------------------------------------------------------------------------

#ifndef WU_PARAMETER_H
#define WU_PARAMETER_H

#include "StdAfx.h"

#include <memory>
#include <map>
#include <string>


namespace wu
{

//-------------------------------------------------------------------------------
class Parameter
{
    friend class std::auto_ptr<Parameter>;

public:
    typedef TCHAR char_type;
    typedef std::basic_string<char_type> string_type;
  
private:
    typedef std::map<string_type, string_type> map_type;

public:
    static Parameter& GetInst();
    static Parameter* GetInstPtr();

    const string_type& GetCmdLine() const;
    unsigned GetArgc() const;
    bool IsOpt(const string_type& a_sParam) const;
    const string_type& GetOpt(const string_type &a_sOpt) const;

private:
    static std::auto_ptr<Parameter> m_pInst;
    string_type m_sCmdLine;
    map_type m_Param;

    Parameter();
    Parameter(const Parameter&) {};
   ~Parameter() {};
};


}  // end of namespace

#endif