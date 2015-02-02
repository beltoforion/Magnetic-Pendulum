// Folgender ifdef-Block ist die Standardmethode zum Erstellen von Makros, die das Exportieren 
// aus einer DLL vereinfachen. Alle Dateien in der DLL werden mit dem MUPARSERLIB_EXPORTS-Symbol
// kompiliert, das in der Befehlszeile definiert wurde. Das Symbol darf nicht für ein Projekt definiert werden,
// das diese DLL verwendet. Alle anderen Projekte, deren Quelldateien diese Datei beinhalten, erkennen 
// MUPARSERLIB_API-Funktionen als aus einer DLL importiert, während die DLL mit diesem Makro
// definierte Symbole als exportiert ansieht.
#ifndef MU_PARSER_DLL_H
#define MU_PARSER_DLL_H

#ifdef MUPARSERLIB_EXPORTS
#define MU_PARSER_API __declspec(dllexport)
#else
#define MU_PARSER_API __declspec(dllimport)
#endif


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef void* parser_handle;
typedef double (*fun_type1)(double); 
typedef double (*fun_type2)(double, double); 
typedef double (*fun_type3)(double, double, double); 
typedef double (*fun_type4)(double, double, double, double); 
typedef double (*fun_type5)(double, double, double, double, double); 
typedef double (*multfun_type)(const double*, int);
typedef double (*strfun_type1)(const char*);
typedef void (*errhandler_type)();
typedef double* (*facfun_type)(const char*);
typedef bool (*identfun_type)(const char*, int&, double&);

extern "C"
{

// Basic operations / initialization  
MU_PARSER_API parser_handle mupInit();
MU_PARSER_API void mupRelease(parser_handle a_hParser);
MU_PARSER_API const char* mupGetExpr(parser_handle a_hParser);
MU_PARSER_API void mupSetExpr(parser_handle a_hParser, const char *a_szExpr);
MU_PARSER_API void mupSetErrorHandler(errhandler_type a_pErrHandler);
MU_PARSER_API void mupSetVarFactory(parser_handle a_hParser, facfun_type a_pFactory);

MU_PARSER_API double mupEval(parser_handle a_hParser);

// Defining callbacks / variables / constants
MU_PARSER_API void mupDefineFun1(parser_handle a_hParser, const char *a_szName, fun_type1 a_pFun, bool a_bAllowOpt = true);
MU_PARSER_API void mupDefineFun2(parser_handle a_hParser, const char *a_szName, fun_type2 a_pFun, bool a_bAllowOpt = true);
MU_PARSER_API void mupDefineFun3(parser_handle a_hParser, const char *a_szName, fun_type3 a_pFun, bool a_bAllowOpt = true);
MU_PARSER_API void mupDefineFun4(parser_handle a_hParser, const char *a_szName, fun_type4 a_pFun, bool a_bAllowOpt = true);
MU_PARSER_API void mupDefineFun5(parser_handle a_hParser, const char *a_szName, fun_type5 a_pFun, bool a_bAllowOpt = true);
MU_PARSER_API void mupDefineStrFun(parser_handle a_hParser, const char *a_szName, strfun_type1 a_pFun, bool a_bAllowOpt = true);
MU_PARSER_API void mupDefineMultFun(parser_handle a_hParser, const char *a_szName, multfun_type a_pFun, bool a_bAllowOpt = true);
MU_PARSER_API void mupDefineOprt(parser_handle a_hParser, const char *a_szName, fun_type2 a_pFun, int a_iPri = 0, bool a_bAllowOpt = true);
MU_PARSER_API void mupDefineConst(parser_handle a_hParser, const char *a_szName, double a_fVal);
MU_PARSER_API void mupDefineStrConst(parser_handle a_hParser, const char *a_szName, const char *a_sVal);
MU_PARSER_API void mupDefineVar(parser_handle a_hParser, const char *a_szName, double *a_fVar);
MU_PARSER_API void mupDefinePostfixOprt(parser_handle a_hParser, const char *a_szName, fun_type1 a_pOprt, bool a_bAllowOpt = true);
MU_PARSER_API void mupDefineInfixOprt(parser_handle a_hParser, const char *a_szName, fun_type1 a_pOprt, bool a_bAllowOpt=true);

// Define character sets for identifiers
MU_PARSER_API void mupDefineNameChars(parser_handle a_hParser, const char *a_szCharset);
MU_PARSER_API void mupDefineOprtChars(parser_handle a_hParser, const char *a_szCharset);
MU_PARSER_API void mupDefineInfixOprtChars(parser_handle a_hParser, const char *a_szCharset);

// Remove all / single variables
MU_PARSER_API void mupRemoveVar(parser_handle a_hParser, const char *a_szName);
MU_PARSER_API void mupClearVar(parser_handle a_hParser);
MU_PARSER_API void mupClearConst(parser_handle a_hParser);
MU_PARSER_API void mupClearOprt(parser_handle a_hParser);

// Querying variables / expression variables / constants
MU_PARSER_API int mupGetExprVarNum(parser_handle a_hParser);
MU_PARSER_API int mupGetVarNum(parser_handle a_hParser);
MU_PARSER_API int mupGetConstNum(parser_handle a_hParser);
MU_PARSER_API void mupGetExprVar(parser_handle a_hParser, unsigned a_iVar, const char **a_pszName, double **a_pVar);
MU_PARSER_API void mupGetVar(parser_handle a_hParser, unsigned a_iVar, const char **a_pszName, double **a_pVar);
MU_PARSER_API void mupGetConst(parser_handle a_hParser, unsigned a_iVar, const char **a_pszName, double &a_pVar);

// Add value recognition callbacks
MU_PARSER_API void mupAddValIdent(parser_handle a_hParser, identfun_type);

// Error handling
MU_PARSER_API bool mupError();
MU_PARSER_API void mupErrorReset();
MU_PARSER_API const char* mupGetErrorMsg();
MU_PARSER_API int mupGetErrorCode();
MU_PARSER_API int mupGetErrorPos();
MU_PARSER_API const char* mupGetErrorToken();

} // extern "C"

#endif