
// StegoApp.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CStegoAppApp:
// Сведения о реализации этого класса: StegoApp.cpp
//

class CStegoAppApp : public CWinApp
{
public:
	CStegoAppApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CStegoAppApp theApp;
