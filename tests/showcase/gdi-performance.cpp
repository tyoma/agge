#include "stdafx.h"
#include "gdi-performance.h"
#include "MainFrm.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Gdiplus;

BEGIN_MESSAGE_MAP(CGDIPerformanceApp, CWinApp)
END_MESSAGE_MAP()

CGDIPerformanceApp::CGDIPerformanceApp()
{
	SetAppID(_T("gdi-performance.AppID.NoVersion"));
}

CGDIPerformanceApp theApp;

BOOL CGDIPerformanceApp::InitInstance()
{
	GdiplusStartupInput gdiplusStartupInput;
   GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);

	CWinApp::InitInstance();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	return TRUE;
}

int CGDIPerformanceApp::ExitInstance()
{
	GdiplusShutdown(_gdiplusToken);
	return CWinApp::ExitInstance();
}
