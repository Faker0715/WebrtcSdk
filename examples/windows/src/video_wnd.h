#ifndef WIN_DEMO_VIDEO_WND_H_
#define WIN_DEMO_VIDEO_WND_H_

#include <mutex>

typedef HANDLE HDRAWDIB;

// Video buffer class
class CVideoBuffer {
public:
    CVideoBuffer();
    ~CVideoBuffer();

    BYTE* GetBuffer(int w = 0, int h = 0);
    void ReleaseBuffer();
    void CleanUpBuffer();
    int stride() const { return (m_w * 3 + 3) / 4 * 4; }
    int width() const { return m_w; }
    int height()const { return m_h; }

protected:
    void* m_buf;
    int m_w;
    int m_h;
    std::mutex m_mutex;
};

class Win32Window {
public:
    Win32Window();
    virtual ~Win32Window();

    HWND handle() const { return wnd_; }

    bool Create(HWND parent, const wchar_t* title, DWORD style, DWORD exstyle,
        int x, int y, int cx, int cy);
    void Destroy();

    // Call this when your DLL unloads.
    static void Shutdown();

protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam,
        LRESULT& result);

    virtual bool OnClose() { return true; }
    virtual void OnNcDestroy() { }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
        LPARAM lParam);

    HWND wnd_;
    static HINSTANCE instance_;
    static ATOM window_class_;
};

class CVideoWnd : public Win32Window {
public:
    CVideoWnd();
    virtual ~CVideoWnd();

    static bool SetDefaultImage(const char* path);

    // implement from IVideoWnd
    BYTE* GetBuffer(int w, int h);
    BOOL ReleaseBuffer();

    void Clear();
    bool SetVisible(bool bShow);
    bool GetVisible();
    /*
    ���RGB.
    @arg pRgb ԭʼRGB����
    @arg width ԭʼ���
    @arg height ԭʼ�߶�
    @arg orgin ��ת���,Ϊ����
    - 0 ����ת
    - 1 90��
    - 2 180��
    - 3 270��
    @return BOOL
    @retval
    */
    BOOL FillBuffer(BYTE* pRgb, int width, int height, int orgin);
    BOOL FillBuffer(BYTE* pRgb, int width, int height) {
        return FillBuffer(pRgb, width, height, m_nDefOrgin);
    }
    // ����ȱʡ��ת����
    void SetDefOrgin(int orgin) { m_nDefOrgin = orgin; }

    // ���ô�����Ӧģʽ
    void SetFitMode(BOOL bFit);
    void SetBkColor(COLORREF color);
    BOOL TaskSnap(LPCTSTR path);
    /*
    �����ı�.
    @arg text ����
    @arg clr ��ɫ
    @arg mode ����ģʽ
    */
    void SetText(const char* text, COLORREF clr, DWORD mode = DT_BOTTOM | DT_SINGLELINE);
    void Refresh();

    // Attributes
public:
    // Operations
public:

    // Implementation
    BOOL m_bShowVol;
    void SetPoint(std::vector<POINT>& pts);
protected:
    std::vector<POINT> points;
    int m_nVol;
    BOOL m_bUseGDI;
    // ����ɫ
    COLORREF m_clrBK;
    // ��Ӧ����
    BOOL m_bFit;
    // ȱʡ��ת����
    int m_nDefOrgin;

    // �ı�
    std::string m_szText;
    // ��ʽ DrawText��ʽ
    DWORD m_nTextFmt;
    // �ı���ɫ
    COLORREF m_clrText;

    BOOL m_bSizeChanged;

    BITMAPINFO m_bmi;
    HDRAWDIB m_hDrawDIB;

    CVideoBuffer m_buffer;

    // Generated message map functions
protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);
    BOOL OnEraseBkgnd(HDC pDC);
};


#endif // WIN_DEMO_VIDEO_WND_H_