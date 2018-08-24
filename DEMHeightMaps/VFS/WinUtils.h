#ifndef WIN_UTILS_H
#define WIN_UTILS_H

#ifdef _WIN32

#include <windows.h>

#include "./OSUtils.h"

#include "../Strings/MyStringAnsi.h"



class WinUtils : public OSUtils
{
	public:

		MyStringAnsi GetDocumentsDataPath() const override;
		MyStringAnsi GetDocumentsFilePath(const MyStringAnsi & file) const override;

        MyStringAnsi GetInstallDataPath() const override;
        MyStringAnsi GetInstallFilePath(const MyStringAnsi & file) const override;

    
		int GetScreenDPI() const override;
		int GetBaseFontSize() const override;


		void OpenURL(const MyStringAnsi & url) const override;
		
		static HWND GetMainWindow(unsigned long processId);
		static bool IsMainWindow(HWND handle);
		
	private:
		static BOOL CALLBACK EnumWindowsCallaback(HWND handle, LPARAM lParam);

};

#endif

#endif
