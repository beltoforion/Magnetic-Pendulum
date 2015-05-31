#include "stdafx.h"
#include "SimPend.h"

//--- Standard includes ---------------------------------------------------------------------
#include <sstream>
#include <limits>
#include <fstream>

//--- Microsofts ----------------------------------------------------------------------------
#include <atlimage.h>
#include <Gdiplusimaging.h>

//--- Utility classes -----------------------------------------------------------------------
#include "utils/muGeneric.h"
#include "utils/suStringTokens.h"
#include "utils/suUtility.h"
#include "utils/auThreads.h"
#include "utils/utMemory.h"
#include "utils/utWideExceptions.h"
#include "utils/muParserDLL.h"

//--- My includes ---------------------------------------------------------------------------
#include "WndOpenGL.h"

//-------------------------------------------------------------------------------------------
// undefines stupid Microsoft macros
#if defined(min) || defined(max)
#undef min
#undef max
#endif


#define _USE_MATH_DEFINES

//-------------------------------------------------------------------------------------------
//
//
//  Simulation data clas
//
//
//-------------------------------------------------------------------------------------------

SimImpl::SimImpl(CWndOpenGL *pWnd, const au::IniFile &iniFile)
:m_pWnd(pWnd)
, m_nRows(0)
, m_nCols(0)
, m_nWinWidth(0)
, m_nWinHeight(0)
, m_nThreads(0)
, m_nMinSteps(0)
, m_nMaxSteps(0)
, m_nBatchMode(0)
, m_fTimeStep(0)
, m_fAbortVel(0)
, m_fFriction(0)
, m_fSimWidth(0)
, m_fSimHeight(0)
, m_fHeight(0)
, m_fMaxTraceLen(0)
, m_fTraceLenBuf(0)
, m_vpSrc()
, m_IdxField()
, m_LenField()
, m_bColorNormalize(true)
, m_LineMgr()
, m_hParser(NULL)
{
    assert(m_pWnd);
    m_hParser = mupCreate(muBASETYPE_FLOAT);

    const wchar_t *szVer = mupGetVersion(m_hParser);

    // bin muParser variables
    mupDefineVar(m_hParser, _T("len"), (double*)&m_fTraceLenBuf);
    mupDefineVar(m_hParser, _T("max_len"), &m_fMaxTraceLen);

    if (mupError(m_hParser))
    {
        throw utils::wruntime_error(mupGetErrorMsg(m_hParser));
    }

    InitFromFile(iniFile);
}

//-------------------------------------------------------------------------------------------
SimImpl::~SimImpl()
{
    mupRelease(m_hParser);
    utils::clear_cont_of_ptr(m_vpSrc);
}

//-------------------------------------------------------------------------------------------
void SimImpl::SetField(int cols, int rows)
{
    m_nRows = rows;
    m_nCols = cols;
    m_IdxField.Resize(m_nCols, m_nRows);
    m_LenField.Resize(m_nCols, m_nRows);

    // Mark as uncalculated
    m_IdxField = -1;

    // Indices of lines waiting for calculation
    m_LineMgr.Reset(m_nRows);
}

//-------------------------------------------------------------------------------------------
/** \brief Query index of next line to calculate and check if a new backup is necessary. */
int SimImpl::QueryNextLine(int &nLine)
{
    int nIdx(0);
    nLine = m_LineMgr.GetNextLine(nIdx);
    return nIdx;
}

//-------------------------------------------------------------------------------------------
/** \brief Convert Window coordinates to physical coordinates. */
void SimImpl::WinCoordToModel(int x, int y, double &sim_x, double &sim_y) const
{
    sim_x = (double)x / (double)m_nWinWidth * m_fSimWidth;
    sim_y = (double)y / (double)m_nWinHeight * m_fSimHeight;
}

//-------------------------------------------------------------------------------------------
void SimImpl::GridCoordToModel(int x, int y, double &sim_x, double &sim_y) const
{
    sim_x = (double)x / (double)m_nCols * m_fSimWidth;
    sim_y = (double)y / (double)m_nRows * m_fSimHeight;
}

//-------------------------------------------------------------------------------------------
void SimImpl::ModelCoordToWin(double sim_x, double sim_y, int &x, int &y) const
{
    x = mu::round((double)sim_x / (double)m_fSimWidth * m_nCols);
    y = mu::round((double)sim_y / (double)m_fSimHeight * m_nRows);
}

//-------------------------------------------------------------------------------------------
/** \brief Mark a line as processed.

  Processed lines must be marked with -1.
  */
void SimImpl::FlagAsDone(int hLine)
{
    m_LineMgr.FlagAsCalculated(hLine);
}

//-------------------------------------------------------------------------------------------
bool SimImpl::IsDone() const
{
    return m_LineMgr.IsDone();
}

//-------------------------------------------------------------------------------------------
int SimImpl::GetBatchMode() const
{
    return m_nBatchMode;
}

//-------------------------------------------------------------------------------------------
// throws runtime_error
void SimImpl::InitFromFile(const au::IniFile &iniFile)
{
    // Set up dimensions and thread count
    int rows(iniFile.GetAsInt(_T("FIELD"), _T("COLS"))),
        cols(iniFile.GetAsInt(_T("FIELD"), _T("ROWS")));
    SetField(cols, rows);

    // Preview window dimensions
    m_nWinWidth = iniFile.GetAsInt(_T("FIELD"), _T("WIN_HEIGHT"), rows);
    m_nWinHeight = iniFile.GetAsInt(_T("FIELD"), _T("WIN_WIDTH"), cols);

    // Simulation field size
    m_fSimWidth = iniFile.GetAsFloatFromExpr(_T("FIELD"), _T("SIM_WIDTH"), (double)cols);
    m_fSimHeight = iniFile.GetAsFloatFromExpr(_T("FIELD"), _T("SIM_HEIGHT"), (double)rows);

    // other parameters
    m_nBatchMode = iniFile.GetAsInt(_T("SIMULATION"), _T("BATCH_MODE"), 0);
    m_nThreads = iniFile.GetAsInt(_T("SIMULATION"), _T("THREADS"), -1);
    m_nMaxSteps = iniFile.GetAsInt(_T("SIMULATION"), _T("MAX_STEPS"));
    m_nMinSteps = iniFile.GetAsInt(_T("SIMULATION"), _T("MIN_STEPS"));
    m_fHeight = iniFile.GetAsFloatFromExpr(_T("SIMULATION"), _T("PEND_HEIGHT"));
    m_fAbortVel = iniFile.GetAsFloatFromExpr(_T("SIMULATION"), _T("ABORT_VEL"));
    m_fTimeStep = iniFile.GetAsFloatFromExpr(_T("SIMULATION"), _T("DELTA_T"));

    std::wstring sExpr(iniFile.GetAsString(_T("SIMULATION"), _T("COLOR_SCHEME")));
    if (!sExpr.length())
    {
        throw utils::wruntime_error(_T("No expression for color scheme given."));
    }
    else
    {
        mupSetExpr(m_hParser, sExpr.c_str());
    }


    m_fFriction = iniFile.GetAsFloatFromExpr(_T("SIMULATION"), _T("FRICTION"));
    if (m_fFriction <= 0)
    {
        throw utils::wruntime_error(_T("Friction parameter must be greater then zero."));
    }


    // set up magnet positions and strengthes
    for (int i = 1;; ++i)
    {
        std::wstringstream ss;
        ss << _T("SOURCE ") << i;
        bool bStat(iniFile.HasSection(ss.str()));
        if (!bStat) // Section does not exist
            break;

        m_vpSrc.push_back(ReadSourceData(ss.str(), iniFile));
    }
}

//-------------------------------------------------------------------------------------------
ISource* SimImpl::ReadSourceData(const std::wstring &sSection, const au::IniFile &iniFile)
{
    using std::sin;
    using std::cos;

    mu::vec2d_type pos;
    double mult(0), size(0);
    int r(0), g(0), b(0);

    // 0.) Color
    std::wstring sCol(iniFile.GetAsString(sSection, _T("COLOR")));
    int stat = swscanf(sCol.c_str(), _T("%d,%d,%d"), &r, &g, &b);
    if (stat != 3)
        throw utils::wruntime_error(_T("Invalid color format (r,g,b is expected)."));

    // 1.) Source strength and size 
    mult = iniFile.GetAsFloatFromExpr(sSection, _T("MULT"));
    size = iniFile.GetAsFloatFromExpr(sSection, _T("SIZE"));

    // 2.) The source type
    std::wstring sType(su::to_upper(iniFile.GetAsString(sSection, _T("TYPE"))));

    // 3.) Position
    if (iniFile.HasKey(sSection, _T("XPOS")) && iniFile.HasKey(sSection, _T("YPOS")))
    {
        pos[0] = iniFile.GetAsFloatFromExpr(sSection, _T("XPOS"));
        pos[1] = iniFile.GetAsFloatFromExpr(sSection, _T("YPOS"));
    }
    else if (iniFile.HasKey(sSection, _T("RAD")) && iniFile.HasKey(sSection, _T("THETA")))
    {
        double rad = iniFile.GetAsFloatFromExpr(sSection, _T("RAD")),
            theta = iniFile.GetAsFloatFromExpr(sSection, _T("THETA"));
        pos[0] = (int)(m_fSimWidth / 2.0 + (double)rad * std::cos((double)theta * mu::DEG2RAD));
        pos[1] = (int)(m_fSimHeight / 2.0 + (double)rad * std::sin((double)theta * mu::DEG2RAD));
    }
    else
    {
        std::wstringstream msg;
        msg << _T("INI format error in section: ") << sSection << _T("\n");
        msg << _T("Required keys: \"XPOS\" and \"YPOS\" or \"RAD\" and \"THETA\" not found.");
        throw utils::wruntime_error(msg.str());
    }

    ISource::SData src;
    src.mult = mult;
    src.size = size;
    src.pos = pos;
    src.r = r;
    src.g = g;
    src.b = b;
    return ISource::CreateSource(sType, src);
}


//-------------------------------------------------------------------------------------------
void SimImpl::QuerySimDim(double &width, double &height) const
{
    width = m_fSimWidth;
    height = m_fSimHeight;
}


//-------------------------------------------------------------------------------------------
void SimImpl::QueryWinDim(int &width, int &height) const
{
    width = m_nWinWidth;
    height = m_nWinHeight;
}

//-------------------------------------------------------------------------------------------
void SimImpl::QuerySimGrid(int &cols, int &rows) const
{
    cols = m_nCols;
    rows = m_nRows;
}

//-------------------------------------------------------------------------------------------
int SimImpl::GetThreadCount() const
{
    return m_nThreads;
}

//-------------------------------------------------------------------------------------------
std::size_t SimImpl::GetSrcCount() const
{
    return m_vpSrc.size();
}

//-------------------------------------------------------------------------------------------
const ISource* SimImpl::GetMagnet(std::size_t idx) const
{
    assert(idx < m_vpSrc.size());
    return m_vpSrc[idx];
}

//-------------------------------------------------------------------------------------------
void SimImpl::CreateBitmap(const std::wstring &sFile)
{
    unsigned *rgbData(new unsigned[m_nCols * m_nRows]);
    memset(rgbData, 0, m_nRows * m_nCols * sizeof(unsigned));

    for (int x = 0; x < m_nCols; ++x)
    {
        for (int y = 0; y < m_nRows; ++y)
        {
            int nIdx(m_IdxField[y][x]);
            if (nIdx < 0)
                continue;

            ASSERT(nIdx < (int)m_vpSrc.size());
            const ISource *pSrc(m_vpSrc[nIdx]);

            // calculate the scalig
            m_fTraceLenBuf = m_LenField[y][x];
            double scale(mupEval(m_hParser));
            rgbData[y * m_nCols + x] = RGB(scale * pSrc->GetBlue(),
                scale * pSrc->GetGreen(),
                scale * pSrc->GetRed());
        }
    }

    CBitmap bitmap;
    bitmap.CreateBitmap(m_nCols, m_nRows, 1, 32, rgbData);

    CImage image;
    image.Attach(bitmap);
    image.Save(sFile.c_str(), Gdiplus::ImageFormatBMP);

    delete[] rgbData;
}

//-------------------------------------------------------------------------------------------
/** \brief Save the state of the calculation to file.

  For simplicity saving is done in alternative data streams of the same file. You should know
  that copying to non ntfs will kill these data streams.
  */
void SimImpl::DumpToFile(const std::wstring &sPath, const std::wstring &sFile)
{
    std::wstring sOutDir(sPath + sFile + _T(".restore"));
    std::wstring sImgFile(sPath.length() ? sPath + _T("\\") + sFile + _T(".bmp") :
        sFile + _T(".bmp"));

    CreateBitmap(sImgFile.c_str());
    CreateDirectory(sOutDir.c_str(), NULL);

    m_IdxField.Write(sOutDir + _T("\\") + sFile + _T(".idx"));
    m_LenField.Write(sOutDir + _T("\\") + sFile + _T(".len"));
    m_LineMgr.SaveState(sOutDir + _T("\\") + sFile + _T(".pos"));
}


//-------------------------------------------------------------------------------------------
/** \brief Restore a calculation from file.
    \param sName Name of the data file.
    */
void SimImpl::Restore(const std::wstring &sPath, const std::wstring &sName)
{
    try
    {
        std::wstring sRetoreDir(sPath + sName + _T(".restore"));

        // Reset data buffer
        m_IdxField.Nullify();
        m_LenField.Nullify();

        // Read data buffer
        m_IdxField.Read(sRetoreDir + _T("\\") + sName + _T(".idx"));
        m_LenField.Read(sRetoreDir + _T("\\") + sName + _T(".len"));
        m_fMaxTraceLen = m_LenField.Max();

        // Read buffer with processed lines
        m_LineMgr.RestoreState(sRetoreDir + _T("\\") + sName + _T(".pos"));

        // Display the current state
        for (int i = 0; i <= m_LineMgr.GetNumLinesDone(); ++i)
            DrawSingleLine(i);
    }
    catch (...)
    {
        // This is most likeley a file does not exist error
        // thats ok, if no restore file exists we start a new calculation
        m_IdxField.Nullify();
        m_LenField.Nullify();
    }

    DrawModel();
}

//-------------------------------------------------------------------------------------------
void SimImpl::ScreenRefresh(int line) const
{
    if (line >= m_nCols || line < 0)
        return;

    if (m_bColorNormalize)
    {
        // We have a new maximum trace length, wen need to recolor all 
        // pixels calculated so far
        for (int y = 0; y < line; ++y)
            DrawSingleLine(y);
    }
    else
        DrawSingleLine(line);
}

//-------------------------------------------------------------------------------------------
void SimImpl::DrawSingleLine(int y) const
{
    if (y <= 0 || y >= m_nRows)
        return;

    assert(m_fMaxTraceLen);

    // buffer variables for muParser
    static double len(1), max_len(1);

    for (int x = 0; x < m_nCols; ++x)
    {
        assert(y < (int)m_IdxField.SizeRow());
        assert(x < (int)m_IdxField.SizeCol());
        int nMag(m_IdxField[y][x]);
        m_fTraceLenBuf = m_LenField[y][x];

        if (nMag >= 0)
        {
            if (nMag >= (int)GetSrcCount())
            {
                throw utils::wruntime_error(_T("Source index out of bounds (config file does not match)."));
            }

            // Use the color scheme expression submitted in the ini file
            const ISource *pSrc(GetMagnet(nMag));
            double scale(mupEval(m_hParser));
            if (mupError(m_hParser))
            {
                std::wstringstream ss;
                ss << _T("[SIMULATION]/COLOR_SCHEME expression error:\n") << mupGetErrorMsg(m_hParser);
                throw utils::wruntime_error(ss.str().c_str());
            }

            m_pWnd->PutPixel(x,
                y,
                (GLubyte)(scale * pSrc->GetRed()),
                (GLubyte)(scale * pSrc->GetGreen()),
                (GLubyte)(scale * pSrc->GetBlue()), 1);
        }
    }
}

//-------------------------------------------------------------------------------------------
void SimImpl::DrawTrace(const std::vector<mu::vec2d_type> &vStrip, int idx) const
{
    ASSERT(m_pWnd);

    CWndOpenGL::PaintLock lock(m_pWnd);
    DrawModel();

    if (idx >= 0)
    {
        const ISource *pMag(m_vpSrc[idx]);
        glColor3ub((GLubyte)pMag->GetRed(),
            (GLubyte)pMag->GetGreen(),
            (GLubyte)pMag->GetBlue());
    }
    else
        glColor3ub((GLubyte)255, (GLubyte)255, (GLubyte)255);

    m_pWnd->DrawLineStrip(vStrip);
}

//-------------------------------------------------------------------------------------------
void SimImpl::DrawModel() const
{
    CWndOpenGL::PaintLock lock(m_pWnd);

    glColor3ub(35, 105, 135);
    glClear(GL_COLOR_BUFFER_BIT);

    // Data
    m_pWnd->DrawFrameBuf();

    for (std::size_t i = 0; i < GetSrcCount(); ++i)
    {
        const ISource* pSrc(GetMagnet(i));
        glColor3ub((GLubyte)pSrc->GetRed(),
            (GLubyte)pSrc->GetGreen(),
            (GLubyte)pSrc->GetBlue());
        if (pSrc->GetType() == ISource::tpLIN)
        {
            m_pWnd->DrawCross((int)pSrc->GetPos()[0],
                (int)pSrc->GetPos()[1],
                (int)std::max((int)pSrc->GetSize(), 5));
        }
        else
        {
            m_pWnd->DrawCircle((int)pSrc->GetPos()[0],
                (int)pSrc->GetPos()[1],
                (int)pSrc->GetSize());
        }
    } // for all sources
}


//-------------------------------------------------------------------------------------------
/** \brief Calculate Pendulum movement for a given start position.
    \param pos 2D vector containing the start position.

    */
int SimImpl::Calc(const mu::vec2d_type &start_pos,
    const mu::vec2d_type &start_vel,
    trace_buf_type *pvTrace)
{
    using std::sqrt;
    using mu::sqr;

    mu::vec2d_type pos(start_pos),  // Pendulum position 
        vel(start_vel),  // Pendulum velovity
        r(0, 0),          // position vector
        acc0(0, 0),       // Pendulum acceleration 
        acc1(0, 0),       // Pendulum acceleration in next time step
        acc2(0, 0),       // Pendulum acceleration in previous time step
        force(0, 0);
    // Proxy pointer for fast array exchange
    mu::vec2d_type *tmp(NULL),
        *acc_p(&acc0), // previous
        *acc(&acc1),   // current
        *acc_n(&acc2); // next
    double t(0), dt(m_fTimeStep), len(0);
    const std::size_t src_num(m_vpSrc.size());
    int closest_src(-1);

    if (pvTrace)
        pvTrace->clear();

    bool  bRunning(true);
    for (int ct = 0; ct < m_nMaxSteps && bRunning; ++ct)
    {
        assert(acc_p);
        assert(acc);
        assert(acc_n);

        // compute new position
        t += dt;
        pos[0] += vel[0] * dt + sqr(dt) * (2.0 / 3.0 * (*acc)[0] - 1.0 / 6.0 * (*acc_p)[0]);
        pos[1] += vel[1] * dt + sqr(dt) * (2.0 / 3.0 * (*acc)[1] - 1.0 / 6.0 * (*acc_p)[1]);

        if (pvTrace && ct % 10 == 0)
            pvTrace->push_back(pos);

        // Reset accelleration
        (*acc_n) = 0.0;

        // Calculate Force, we deal with Forces proportional
        // to the distance or the inverse square of the distance
        double closest_dist(std::numeric_limits<double>::max());
        for (std::size_t i = 0; i < src_num; ++i)
        {
            const ISource* const pSrc(m_vpSrc[i]);
            const double dist(sqrt(sqr(pSrc->GetPos()[0] - pos[0]) +
                sqr(pSrc->GetPos()[1] - pos[1]) +
                sqr(m_fHeight)));

            // Determine closest source index and distance
            if (dist < closest_dist)
            {
                closest_src = (int)i;
                closest_dist = dist;
            }

            // position vetor
            r[0] = pos[0] - pSrc->GetPos()[0];
            r[1] = pos[1] - pSrc->GetPos()[1];

            pSrc->QueryForce(force, r, dist);
            (*acc_n)[0] -= force[0];
            (*acc_n)[1] -= force[1];

            // Check for end condition
            if (ct > m_nMinSteps && abs(r) < pSrc->GetSize() && abs(vel) < m_fAbortVel)
            {
                bRunning = false;
                continue;
            }
        } // for (all Magnets)

        //--------------------------------------------------------------
        // 3.) We have now the acceleration vector containing the influence of all 
        //     forcefied sources, now we need to apply friction proporional to the
        //     velocity.
        (*acc_n)[0] -= vel[0] * m_fFriction;
        (*acc_n)[1] -= vel[1] * m_fFriction;

        //--------------------------------------------------------------
        // 4.) We are almost done, finally we need to compute the new velocity
        //     using Beeman integration
        //     
        //     http://en.wikipedia.org/wiki/Beeman%27s_algorithm#Equation
        //    
        vel[0] += dt * (1.0 / 3.0 * (*acc_n)[0] + 5.0 / 6.0 * (*acc)[0] - 1.0 / 6.0 * (*acc_p)[0]);
        vel[1] += dt * (1.0 / 3.0 * (*acc_n)[1] + 5.0 / 6.0 * (*acc)[1] - 1.0 / 6.0 * (*acc_p)[1]);

        //--------------------------------------------------------------
        // 5.) flip the acc buffer
        tmp = acc_p;
        acc_p = acc;
        acc = acc_n;
        acc_n = tmp;

        len += abs(vel);
    }  // for (trace pendulum movement)


    // store the data, thread safety should not be an issue here
    // no two threads will write the same line, no buffer changes...
    int x(0), y(0);
    ModelCoordToWin(start_pos[0], start_pos[1], x, y);
    if (x < 0 || x >= (int)m_IdxField.SizeCol())
        return -1;

    if (y<0 || y >= (int)m_IdxField.SizeRow())
        return -1;

    m_IdxField[y][x] = closest_src;
    m_LenField[y][x] = len;

    // Place write access to members behind in the next scope:
    if (len>m_fMaxTraceLen)
    {
        m_fMaxTraceLen = len;
        m_bColorNormalize = true;
    }
    else
        m_bColorNormalize = false;

    return closest_src;
}

