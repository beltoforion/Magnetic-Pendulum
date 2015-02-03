#ifndef SIM_PEND_H
#define SIM_PEND_H

#include <afxmt.h>          // for Threading support

#include <vector>
#include <string>
#include "utils/auIniFile.h"
#include "utils/muGeneric.h"
#include "utils/muVector.h"
#include "utils/muBlockMatrix.h"
#include "utils/muParserDLL.h"
#include "Source.h"
#include "TaskMgr.h"


//---------------------------------------------------------------------------------------
// Forward declarations
class CWndOpenGL;

//---------------------------------------------------------------------------------------
/** \brief Simulation implementation.

  Implements the pendulum and magnets simulation.
  */
class SimImpl
{
public:
    typedef mu::BlockMatrix<int> int_field_type;
    typedef mu::BlockMatrix<double> float_field_type;
    typedef std::vector< ISource* > source_buf_type;
    typedef std::vector< mu::vec2d_type > trace_buf_type;

    SimImpl(CWndOpenGL *pWnd, const au::IniFile &iniFile);
    virtual ~SimImpl();

    void QuerySimDim(double &width, double &height) const;
    void QueryWinDim(int &width, int &height) const;
    void QuerySimGrid(int &cols, int &rows) const;
    void WinCoordToModel(int x, int y, double &sim_x, double &sim_y) const;
    void GridCoordToModel(int x, int y, double &sim_x, double &sim_y) const;
    void ModelCoordToWin(double sim_x, double sim_y, int &x, int &y) const;

    int GetThreadCount() const;
    int GetPixSize() const;
    std::size_t GetSrcCount() const;

    void SetField(int cols, int rows);
    int QueryNextLine(int &nLine);
    void FlagAsDone(int hLine);
    bool IsDone() const;

    void InitFromFile(const au::IniFile &iniFile);
    void Restore(const std::wstring &sPath, const std::wstring &sName);
    int Calc(const mu::vec2d_type &start_pos, const mu::vec2d_type &start_vel = mu::vec2d_type(), trace_buf_type *pvTrace = nullptr);
    const ISource* GetMagnet(std::size_t idx) const;

    void DumpToFile(const std::wstring &sPath, const std::wstring &sFile);
    void CreateBitmap(const std::wstring &sFile);

    // Grafical output
    void ScreenRefresh(int line) const;
    void DrawSingleLine(int y) const;
    void DrawModel() const;
    void DrawTrace(const std::vector<mu::vec2d_type> &vStrip, int idx) const;
    int GetBatchMode() const;

private:
    CWndOpenGL *m_pWnd;

    int m_nRows;                    ///< Number of rows in the simulation field 
    int m_nCols;                    ///< Number of Columns in the simulation field.
    int m_nWinWidth;                ///< Width of the preview window
    int m_nWinHeight;               ///< Height of the Preview window
    int m_nThreads;                 ///< Create this many threads for the calculation 
    int m_nMinSteps;
    int m_nMaxSteps;
    int m_nBatchMode;

    double m_fTimeStep;             ///< Integration size (timesteps)
    double m_fAbortVel;             ///< Stop iteration if tracer speed drops below this value.
    double m_fFriction;             ///< Friction coefficient
    double m_fSimWidth;             ///< With of the simulation field
    double m_fSimHeight;            ///< Height of the simulation field
    double m_fHeight;               ///< Height of the Pendulum above the magnets
    double m_fMaxTraceLen;          ///< The maximum length of all traces calculated so far.
    mutable double m_fTraceLenBuf;  ///< I need this as a buffer for muParser; mutable because it doesn't break constness
    bool m_bColorNormalize;

    TaskMgr m_LineMgr;              ///< A class managing the line distribution among the threads.
    muParserHandle_t m_hParser;        ///< Function parser for the color scaling functions
    source_buf_type m_vpSrc;        ///< Sources following columbs law
    int_field_type   m_IdxField;    ///< Result field for magnet indices
    float_field_type m_LenField;    ///< Result field for trace lengths

    SimImpl(const SimImpl &ref);
    SimImpl& operator=(const SimImpl &ref);
    ISource* ReadSourceData(const std::wstring &sSection, const au::IniFile &iniFile);
};

#endif // include guard