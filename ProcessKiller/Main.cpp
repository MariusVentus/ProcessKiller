#include <windows.h>
#include <string>
#include <tlhelp32.h>
#include <vector>
#include <algorithm>


#define IDC_MAIN_EDIT 101
#define ID_FILE_EXIT 9001
#define ID_ABOUT 9002
#define ID_HELP 9003
#define ID_KILLEM 9004


//Globals
const char g_szClassName[] = "myWindowClass";
const char g_WindowTitle[] = "Process Killer V0.0.21";
HWND hMainWindow, hProcessList, hProcessName;
RECT g_MainWin;
int g_ScrollY = 0;
int g_ScrollYSensitivity = 50;

//Forward Declarations
bool RegisterMainWindow(HINSTANCE hInstance);
bool CreateMainWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void AddMenu(HWND hwnd);
void AddControls(HWND hwnd);
void KillProcess(const std::string& processName);
std::string GetProcessList();
void ResetScrollbarSize();



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG Msg;

	if (!RegisterMainWindow(hInstance)) {
		return 0;
	}

	if (!CreateMainWindow(hInstance)) {
		return 0;
	}

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}


bool RegisterMainWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wc = { 0 };

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(100, 0, 0));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = (HICON)LoadImage(hInstance, "Resources\\PK.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
	wc.hIcon = (HICON)LoadImage(hInstance, "Resources\\PK.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return false;
	}
	else {
		return true;
	}
}

bool CreateMainWindow(HINSTANCE hInstance)
{
	hMainWindow = CreateWindowEx(WS_EX_CLIENTEDGE, g_szClassName, g_WindowTitle, WS_OVERLAPPEDWINDOW | WS_VSCROLL,
		CW_USEDEFAULT, CW_USEDEFAULT, 500, 440, NULL, NULL, hInstance, NULL);

	if (hMainWindow == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return false;
	}
	else {
		return true;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		AddMenu(hwnd);
		AddControls(hwnd);
		break;
	case WM_COMMAND:
		switch (wParam)
		{
		case ID_FILE_EXIT:
			PostQuitMessage(0);
			break;
		case ID_ABOUT:
			MessageBox(NULL, "Needed a way to kill processes without PowerShell, CommandLine, or TaskManager. Woo.\n\n-Marius Ventus", "About", MB_OK | MB_ICONINFORMATION);
			break;
		case ID_HELP:
			MessageBox(NULL, "No help, only Zuul.\nOr reaching me on Teams.\n\nOr the Readme:\nhttps://github.com/MariusVentus/ProcessKiller/blob/master/README.md ", "Halp", MB_OK | MB_ICONINFORMATION);
			break;
		case ID_KILLEM:
			//Init
			char rawNote[3000] = "";
			std::string stringNote = "";
			GetWindowText(hProcessName, rawNote, 3000);
			stringNote = rawNote;
			//
			KillProcess(stringNote);
			break;
		}
		break;
	case WM_VSCROLL:
	{
		auto action = LOWORD(wParam);
		HWND hScroll = (HWND)lParam;
		int pos = -1;
		if (action == SB_THUMBPOSITION || action == SB_THUMBTRACK) {
			pos = HIWORD(wParam);
		}
		else if (action == SB_LINEDOWN) {
			pos = g_ScrollY + g_ScrollYSensitivity;
		}
		else if (action == SB_LINEUP) {
			pos = g_ScrollY - g_ScrollYSensitivity;
		}
		if (pos == -1) {
			break;
		}
		//Keeps repeated commands (IE Buttons) from scrolling to infinity
		SCROLLINFO si = { 0 };
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = pos;
		si.nTrackPos = 0;
		SetScrollInfo(hMainWindow, SB_VERT, &si, true);
		GetScrollInfo(hMainWindow, SB_VERT, &si);
		pos = si.nPos;
		//As far as I can tell, this was for compatability somehow, but no idea how. I don't need to know how at this point.
		//POINT pt;
		//pt.x = 0;
		//pt.y = pos - g_scrollY;
		////auto hdc = GetDC(hMainWindow);
		////LPtoDP(hdc, &pt, 1);
		////ReleaseDC(hMainWindow, hdc);
		ScrollWindow(hMainWindow, 0, -(pos - g_ScrollY), NULL, NULL);
		g_ScrollY = pos;
	}
	break;
	case WM_SIZE:
		ResetScrollbarSize();
		break;
	case WM_MOUSEWHEEL:
	{
		int pos = -1;
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
			pos = g_ScrollY - g_ScrollYSensitivity;
			//mouse wheel scrolled up
		}
		else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) {
			pos = g_ScrollY + g_ScrollYSensitivity;
			//mouse wheel scrolled down
		}
		else { //always goes here
			//unknown mouse wheel scroll direction
			break;
		}
		if (pos == -1) {
			break;
		}
		SCROLLINFO si = { 0 };
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = pos;
		si.nTrackPos = 0;
		SetScrollInfo(hMainWindow, SB_VERT, &si, true);
		GetScrollInfo(hMainWindow, SB_VERT, &si);
		pos = si.nPos;
		ScrollWindow(hMainWindow, 0, -(pos - g_ScrollY), NULL, NULL);
		g_ScrollY = pos;
	}
	break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void AddMenu(HWND hwnd)
{
	HMENU hMenu, hFileMenu;
	hMenu = CreateMenu();
	//File Menu
	hFileMenu = CreatePopupMenu();
	AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hFileMenu, MF_STRING, ID_FILE_EXIT, "Exit");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hFileMenu, "File ");

	//Remaining Main Menu Items
	AppendMenu(hMenu, MF_STRING, ID_ABOUT, "About");
	AppendMenu(hMenu, MF_STRING, ID_HELP, "Help");

	SetMenu(hwnd, hMenu);
}

void AddControls(HWND hwnd)
{
	//Logo and Title --------------------------------------------------


	//Main Controls --------------------------------------------------
	CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "Process List", WS_CHILD | WS_VISIBLE,
		15, 15, 440, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
	hProcessList = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", GetProcessList().c_str(),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		15, 40, 440, 200, hwnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);

	//Process Killing
	CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "Process Name (Case Sensitive)", WS_CHILD | WS_VISIBLE,
		15, 250, 440, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
	hProcessName = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "ExampleProcess.exe",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
		15, 275, 440, 25, hwnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);

	//Scrubber, Calculator, Copy to ClipBoard
	CreateWindowEx(WS_EX_CLIENTEDGE, "Button", "Kill Process!", WS_CHILD | WS_VISIBLE,
		15, 310, 440, 50, hwnd, (HMENU)ID_KILLEM, GetModuleHandle(NULL), NULL);
}

void KillProcess(const std::string& processName)
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (strcmp(pEntry.szExeFile, processName.c_str()) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 1);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}

std::string GetProcessList()
{
	//Populate Entity List
	std::vector<std::pair<std::string,std::string>> outputEntries;
	std::string outputList = "";
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	std::string withCaps = "";
	std::string noCaps = "";

	while (hRes)
	{
		//Generate Pair
		withCaps = pEntry.szExeFile;
		noCaps = withCaps;
		for (unsigned i = 0; i < noCaps.size(); i++) {
			noCaps[i] = tolower(noCaps[i]);
		}
		//Emplace Back
		outputEntries.emplace_back(noCaps, withCaps);
		if (outputEntries.back().first.find(".exe") == std::string::npos) {
			outputEntries.pop_back();
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	
	//Sort it
	std::sort(outputEntries.begin(), outputEntries.end());
	
	
	outputList.append(outputEntries[0].second);
	outputList.append("\r\n");
	for (unsigned i = 1; i < outputEntries.size(); i++) {
		if (outputEntries[i] != outputEntries[i - 1]) {
			outputList.append(outputEntries[i].second);
			outputList.append("\r\n");
		}
	}
	outputList.erase(outputList.find_last_of("\r\n"));
	CloseHandle(hSnapShot);

	return outputList;
}

void ResetScrollbarSize()
{
	GetWindowRect(hMainWindow, &g_MainWin);
	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = 440;
	si.nPage = (g_MainWin.bottom - g_MainWin.top);
	si.nPos = 0;
	si.nTrackPos = 0;
	SetScrollInfo(hMainWindow, SB_VERT, &si, true);
}
