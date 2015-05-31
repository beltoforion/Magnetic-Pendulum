#include "stdafx.h"

//--- Standard includes ---------------------------------------------------------------------
#include <cmath>
#include <cassert>
#include <sstream>
#include <stdexcept>

//--- Utility includes ----------------------------------------------------------------------
#include "utils/muGeneric.h"
#include "utils/auThreads.h"
#include "utils/muVector.h"

//-------------------------------------------------------------------------------------------
#include "SimApp.h"
#include "SimThread.h"
#include "WndOpenGL.h"

LPCTSTR CWndOpenGL::m_lpszClassName = NULL;

//-------------------------------------------------------------------------------------------
// COpenGLWin
BEGIN_MESSAGE_MAP(CWndOpenGL, CWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_CLOSE()
END_MESSAGE_MAP()
IMPLEMENT_DYNAMIC(CWndOpenGL, CWnd)

//-------------------------------------------------------------------------------------------
CWndOpenGL::CWndOpenGL()
    :CWnd()
    ,m_pDC(NULL)
    ,m_hRC(NULL)
    ,m_nCols(0)
    ,m_nRows(0)
    ,m_fWidth(0)
    ,m_fHeight(0)
    ,m_nWinWidth(0)
    ,m_nWinHeight(0)
    ,m_ThreadLock()
    ,m_bLocked(false)
    ,m_vFieldData()
    ,m_pSim(NULL)
{}


//-------------------------------------------------------------------------------------------
CWndOpenGL::~CWndOpenGL()
{}


//-------------------------------------------------------------------------------------------
void CWndOpenGL::SetSim(SimThread *pSim)
{
    assert(pSim);
    m_pSim = pSim;
}

//-------------------------------------------------------------------------------------------
void CWndOpenGL::InitGL()
{
    CClientDC clientDC(this);
    // Get the rendering context
    m_hRC = wglCreateContext(clientDC.m_hDC);
    wglMakeCurrent(clientDC.m_hDC, m_hRC);

    // Pixel pack alignment of 1 is important for 
    // glDrawPixels calls. In order to by in sync
    // with m_iFrameBuf size
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    //glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //  glOrtho(0, m_nCols, 0, m_nRows, 0, 1);
    glOrtho(0, m_fWidth, 0, m_fHeight, 0, 1);
}

//---------------------------------------------------------------------------------------
void CWndOpenGL::InitFrameBuf()
{
    m_vFieldData.assign(3 * m_nCols * m_nRows, 0);
}

//---------------------------------------------------------------------------------------
BOOL CWndOpenGL::Create(DWORD dwExStyle,
    DWORD dwStyle,
    const RECT& rect,
    CWnd* pParentWnd,
    UINT /*nID*/,
    CCreateContext* /*pContext*/)
{
    if (m_lpszClassName == NULL)
    {
        m_lpszClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
            AfxGetApp()->LoadStandardCursor(IDC_ARROW));
    }

    // Set Size and scaling information
    m_nWinWidth = rect.right - rect.left;
    m_nWinHeight = rect.bottom - rect.top;

    CRect rc;
    rc.SetRect(CPoint(0, 0), CPoint(m_nWinWidth, m_nWinHeight));
    BOOL bStat = CreateEx(dwExStyle,
        m_lpszClassName,
        _T("Magnets and Pendulum Fractal"),
        dwStyle,
        rc,
        pParentWnd,
        NULL,
        NULL);
    ASSERT(bStat);
    ShowWindow(SW_SHOW);
    return bStat;
}


//---------------------------------------------------------------------------------------
/** \brief Create a new OpenGL window.
    \param a_WinRect Window geometry.
    \param nBackBufCols Columns in the background buffer.
    \param nBackBufRows Rows in the background buffer.
    */
BOOL CWndOpenGL::Create(const CRect &a_WinRect,
    int nCols,
    int nRows,
    double fWidth,
    double fHeight)
{
    DWORD dwExStyle(WS_EX_APPWINDOW),
        dwStyle(WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU);

    m_nCols = nCols;
    m_nRows = nRows;

    m_fWidth = fWidth;
    m_fHeight = fHeight;

    return CWndOpenGL::Create(dwExStyle,
        dwStyle,
        a_WinRect,
        NULL,
        0,
        NULL);
}


//-------------------------------------------------------------------------------------------
int CWndOpenGL::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    CClientDC clientDC(this);

    // Define the pixel format
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL;
    pfd.dwFlags |= PFD_DOUBLEBUFFER;

    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int iPixFmt;
    if ((iPixFmt = ::ChoosePixelFormat(clientDC.m_hDC, &pfd)) == 0)
    {
        ::MessageBox(NULL, _T("ChoosePixelFormat Failed"), NULL, MB_OK);
        return 0;
    }

    if (SetPixelFormat(clientDC.m_hDC, iPixFmt, &pfd) == 0)
    {
        ::MessageBox(NULL, _T("SetPixelFormat Failed"), NULL, MB_OK);
        return 0;
    };

    return 0;
}


//-------------------------------------------------------------------------------------------
void CWndOpenGL::BeginGLPaint()
{
    au::AutoLock<CCriticalSection> lock(&m_ThreadLock);

    if (!m_hWnd)
        return;

    CClientDC clientDC(this);
    wglMakeCurrent(clientDC.m_hDC, m_hRC);
}


//---------------------------------------------------------------------------------------
void CWndOpenGL::PutPixel(int x, int y, GLubyte r, GLubyte g, GLubyte b, int /* size */)
{
    au::AutoLock<CCriticalSection> lock(&m_ThreadLock);

    if (x < 0 || x >= m_nCols || y < 0 || y >= m_nRows || !m_hWnd)
        return;

    int idx(3 * (m_nCols * y + x));
    if (idx < 0 || (idx + 2) >= (int)m_vFieldData.size())
    {
        std::stringstream ss;
        ss << "PutPixel:  m_vFieldData out of range exception (idx="
            << idx << "; bufsize=" << (int)m_vFieldData.size() << ")";
        throw std::out_of_range(ss.str());
    }

    // bypass operator 
    (&m_vFieldData[0])[idx] = r;
    (&m_vFieldData[0])[idx + 1] = g;
    (&m_vFieldData[0])[idx + 2] = b;
}

//---------------------------------------------------------------------------------------
void CWndOpenGL::DrawFrameBuf() const
{
    au::AutoLock<CCriticalSection> lock(&m_ThreadLock);

    if (!m_hWnd)
        return;

    double zoom_x((double)m_nWinWidth / m_nCols),
        zoom_y((double)m_nWinHeight / m_nRows);
    glRasterPos2i(0, 0);
    glPixelZoom((GLfloat)zoom_x, (GLfloat)zoom_y);
    glDrawPixels(m_nCols, m_nRows, GL_RGB, GL_UNSIGNED_BYTE, &m_vFieldData[0]);
}

//-------------------------------------------------------------------------------------------
void CWndOpenGL::DrawCircle(int x, int y, double rad, int how, int steps) const
{
    au::AutoLock<CCriticalSection> lock(&m_ThreadLock);

    using std::sin;
    using std::cos;

    if (!m_hWnd)
        return;

    const double scale(360.0 / (double)steps);

    glBegin(how);

    for (int i = 0; i < steps; ++i)
    {
        const double degInRad(i * scale * mu::DEG2RAD);
        glVertex2f((GLfloat)x + (GLfloat)(cos(degInRad)*rad),
            (GLfloat)y + (GLfloat)(sin(degInRad)*rad));
    }

    glEnd();
}

//-------------------------------------------------------------------------------------------
void CWndOpenGL::DrawCross(int x, int y, double size) const
{
    au::AutoLock<CCriticalSection> lock(&m_ThreadLock);

    if (!m_hWnd)
        return;

    glBegin(GL_LINE_STRIP);
    glVertex2f((GLfloat)x, (GLfloat)(y - size));
    glVertex2f((GLfloat)x, (GLfloat)(y + size));
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex2f((GLfloat)(x - size), (GLfloat)(y));
    glVertex2f((GLfloat)(x + size), (GLfloat)(y));
    glEnd();
}

//-------------------------------------------------------------------------------------------
void CWndOpenGL::DrawLineStrip(const std::vector<mu::vec2d_type> &vStrip, int width) const
{
    au::AutoLock<CCriticalSection> lock(&m_ThreadLock);

    if (!m_hWnd)
        return;

    glLineWidth((GLfloat)width);
    glBegin(GL_LINE_STRIP);

    for (std::size_t i = 0; i < vStrip.size(); ++i)
    {
        const mu::vec2d_type &vec(vStrip[i]);
        glVertex2f((GLfloat)(vec[0]), (GLfloat)(vec[1]));
    }

    glEnd();
}

//-------------------------------------------------------------------------------------------
void CWndOpenGL::EndGLPaint()
{
    au::AutoLock<CCriticalSection> lock(&m_ThreadLock);

    if (!m_hWnd)
        return;

    wglMakeCurrent(NULL, NULL);

    CClientDC clientDC(this);
    SwapBuffers(clientDC.m_hDC);
}

//-------------------------------------------------------------------------------------------
void CWndOpenGL::OnPaint()
{
    au::AutoLock<CCriticalSection> lock(&m_ThreadLock);

    if (!m_hWnd)
        return;

    CPaintDC dc(this); // device context for painting

    CClientDC clientDC(this);
    SwapBuffers(clientDC.m_hDC);
}

//-------------------------------------------------------------------------------------------
/** \brief Save DC to bitmap image.

  Code taken from:  http://www.c-plusplus.de/forum/viewtopic-var-p-is-18276.html
  */
int CWndOpenGL::HDCToBitmap(HDC hdc, int width, int height, const wchar_t *filename) const
{
    au::AutoLock<CCriticalSection> lock(&m_ThreadLock);

    HDC hdc2;
    HBITMAP aBmp;
    BITMAPINFO bi;
    HGDIOBJ OldObj;
    void *dibvalues;
    HANDLE fileHandle;

    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    DWORD bytes_write,
        bytes_written;

    hdc2 = CreateCompatibleDC(hdc);

    ZeroMemory(&bmih, sizeof(BITMAPINFOHEADER));
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biHeight = height;
    bmih.biWidth = width;
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3) * bmih.biHeight;
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrImportant = 0;

    bi.bmiHeader = bmih;

    aBmp = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (void**)&dibvalues, NULL, NULL);

    if (aBmp == NULL)
    {
        OutputDebugString(_T("CreateDIBSection failed!\n"));
        return 0;
    }

    OldObj = SelectObject(hdc2, aBmp);
    BitBlt(hdc2, 0, 0, width, height, hdc, 0, 0, SRCCOPY);

    ZeroMemory(&bmfh, sizeof(BITMAPFILEHEADER));
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
    bmfh.bfSize = (3 * bmih.biHeight*bmih.biWidth) + sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
    bmfh.bfType = 0x4d42;
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;

    fileHandle = CreateFile(filename, 
                            GENERIC_READ | GENERIC_WRITE, 
                            (DWORD)0, 
                            NULL,
                            CREATE_ALWAYS, 
                            FILE_ATTRIBUTE_NORMAL, 
                            (HANDLE)NULL);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        OutputDebugString(_T("CreateFile failed!\n"));
        return 0;
    }

    // Write the BITMAPFILEHEADER
    bytes_write = sizeof(BITMAPFILEHEADER);
    if (!WriteFile(fileHandle, (void*)&bmfh, bytes_write, &bytes_written, NULL))
    {
        OutputDebugString(_T("WriteFile failed!\n"));
        return 0;
    }

    //Write the BITMAPINFOHEADER
    bytes_write = sizeof(BITMAPINFOHEADER);
    if (!WriteFile(fileHandle, (void*)&bmih, bytes_write, &bytes_written, NULL))
    {
        OutputDebugString(_T("WriteFile failed!\n"));
        return 0;
    }

    //Write the Color Index Array???
    bytes_write = bmih.biSizeImage;//3*bmih.biHeight*bmih.biWidth;
    if (!WriteFile(fileHandle, (void*)dibvalues, bytes_write, &bytes_written, NULL))
    {
        OutputDebugString(_T("WriteFile failed!\n"));
        return 0;
    }

    CloseHandle(fileHandle);

    DeleteObject(SelectObject(hdc2, OldObj));
    DeleteDC(hdc2);

    return 1;
}

//-------------------------------------------------------------------------------------------
void CWndOpenGL::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (!m_hWnd)
        return;

    CRect rect;
    GetClientRect(&rect);
    m_pSim->HandleMouseClick(point.x, rect.Height() - point.y);

    CWnd::OnMouseMove(nFlags, point);
}

//-------------------------------------------------------------------------------------------
void CWndOpenGL::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_hWnd)
        return;

    CRect rect;
    GetClientRect(&rect);
    m_pSim->HandleMouseMove(point.x, rect.Height() - point.y);
    CWnd::OnMouseMove(nFlags, point);
}

//-------------------------------------------------------------------------------------------
void CWndOpenGL::OnClose()
{
    m_pSim->Finalize();
    CWnd::OnClose();
}
