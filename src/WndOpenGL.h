#pragma once

#include "utils/muGeneric.h"
#include "utils/muVector.h"

class SimThread;


//-------------------------------------------------------------------------------------------
class CWndOpenGL : public CWnd
{
    friend class PaintLock;

public:
    //-----------------------------------------------------------------------------------------
    class PaintLock
    {
    public:
        PaintLock(CWndOpenGL *pLockObj)
            :m_pLockObj(pLockObj)
            , m_bLockOwner(false)
        {
            m_pLockObj->m_ThreadLock.Lock();
            if (m_pLockObj->m_bLocked == false)
            {
                m_pLockObj->BeginGLPaint();
                m_pLockObj->m_bLocked = true;
                m_bLockOwner = true;
            }
        }

        ~PaintLock()
        {
            if (m_bLockOwner && m_pLockObj->m_bLocked)
            {
                m_pLockObj->m_bLocked = false;
                m_pLockObj->EndGLPaint();
            }
            m_pLockObj->m_ThreadLock.Unlock();
        }

    private:
        CWndOpenGL *m_pLockObj;
        bool m_bLockOwner;
    };

    static LPCTSTR m_lpszClassName;

    CWndOpenGL();
    virtual ~CWndOpenGL();
    virtual BOOL Create(DWORD dwExStyle,
        DWORD dwStyle,
        const RECT& rect,
        CWnd* pParentWnd,
        UINT nID,
        CCreateContext* pContext = nullptr);
    virtual BOOL Create(const CRect &a_Rect,
        int nCols,
        int nRows,
        double fWidth,
        double fHeight);
    void SetSim(SimThread *pSim);
    void BeginGLPaint();
    void EndGLPaint();
    void DrawCircle(int x, int y, double rad, int how = GL_TRIANGLE_FAN, int steps = 36) const;
    void DrawCross(int x, int y, double size) const;
    void DrawLineStrip(const std::vector<mu::vec2d_type> &vStrip, int width = 1) const;
    void DrawFrameBuf() const;
    void PutPixel(int x, int y, GLubyte r, GLubyte g, GLubyte b, int size = 1);

private:
    typedef std::vector<GLubyte> field_buf_type;
    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CWndOpenGL)

    CDC *m_pDC;         ///< Handle to Device context
    HGLRC m_hRC;        ///< Handle to RC
    int m_nWinWidth;    ///< Window width
    int m_nWinHeight;   ///< Window height
    int m_nCols;        ///< Number of Columns
    int m_nRows;        ///< Number of Rows

    double m_fWidth;
    double m_fHeight;

    SimThread *m_pSim;
    mutable CCriticalSection  m_ThreadLock;
    volatile bool m_bLocked;
    field_buf_type m_vFieldData;

    CWndOpenGL(const CWndOpenGL &ref);
    CWndOpenGL& operator=(const CWndOpenGL &ref);

    void InitGL();
    void InitFrameBuf();
    int HDCToBitmap(HDC hdc, int width, int height, const wchar_t *filename) const;

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnClose();
};


