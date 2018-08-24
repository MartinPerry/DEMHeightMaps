#include "./WinUtils.h"

#ifdef _WIN32

#if defined(_MSC_VER) && defined(__clang__)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#endif

#include <ShellScalingAPI.h>
#include <thread>

#include <cstring>
#include <shellapi.h>

#pragma comment(lib, "Shcore.lib")	
#pragma comment(lib, "shell32.lib") 




MyStringAnsi WinUtils::GetDocumentsDataPath() const
{
	return this->info.documentPath;
}

MyStringAnsi WinUtils::GetDocumentsFilePath(const MyStringAnsi & file) const
{
	return this->GetDocumentsDataPath() + MyStringAnsi("/") + file;
}


MyStringAnsi WinUtils::GetInstallDataPath() const
{
	return this->info.installPath;
}

MyStringAnsi WinUtils::GetInstallFilePath(const MyStringAnsi & file) const
{
    return this->GetInstallDataPath() + MyStringAnsi("/") + file;
}

int WinUtils::GetScreenDPI() const
{
	if (this->info.screenDPI != 0)
	{
		return this->info.screenDPI;
	}

	HWND handle = GetMainWindow(GetCurrentProcessId());

	auto monitor = MonitorFromWindow(handle, MONITOR_DEFAULTTOPRIMARY);

	UINT dpiX = 0;
	UINT dpiY = 0;
	GetDpiForMonitor(monitor, MDT_RAW_DPI, &dpiX, &dpiY);

	return static_cast<int>((dpiX + dpiY) * 0.5);
}

int WinUtils::GetBaseFontSize() const
{
	if (this->info.baseFontSize != 0)
	{
		return this->info.baseFontSize;
	}

	HWND handle = GetMainWindow(GetCurrentProcessId());

	//The tmHeight value of the TEXTMETRIC structure thus actually refers to line spacing rather than the font point size. 
	//The point size can be derived from tmHeight minus tmInternalLeading. 

	TEXTMETRIC m;
	GetTextMetrics(GetDC(handle), &m);
	
	return (m.tmHeight - m.tmInternalLeading);
}

void WinUtils::OpenURL(const MyStringAnsi & url) const
{
	//http://stackoverflow.com/questions/13643074/programmatically-opening-multiple-urls-in-default-browser
	//http://www.cplusplus.com/forum/beginner/25699/
	
	ShellExecuteA(GetDesktopWindow(), ("open"), url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}





struct handle_data {
	unsigned long process_id;
	HWND best_handle;
};

bool WinUtils::IsMainWindow(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK WinUtils::EnumWindowsCallaback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !WinUtils::IsMainWindow(handle)) {
		return TRUE;
	}
	data.best_handle = handle;
	return FALSE;
}



HWND WinUtils::GetMainWindow(unsigned long processId)
{
	handle_data data;
	data.process_id = processId;
	data.best_handle = 0;
	EnumWindows(WinUtils::EnumWindowsCallaback, (LPARAM)&data);
	return data.best_handle;
}

#endif
