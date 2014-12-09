#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

class CGDIPerformanceApp : public CWinApp
{
	ULONG_PTR _gdiplusToken;

	DECLARE_MESSAGE_MAP()

	virtual BOOL InitInstance();
	virtual int ExitInstance();

public:
	CGDIPerformanceApp();
};

extern CGDIPerformanceApp theApp;
