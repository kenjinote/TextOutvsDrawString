#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib,"gdiplus")
#pragma comment(lib,"comctl32")

#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>

using namespace Gdiplus;

TCHAR szClassName[] = TEXT("Window");

int CALLBACK EnumFontFamProc(const LOGFONT FAR* lpelf, const TEXTMETRIC FAR* lpntm, DWORD FontType, LPARAM lParam)
{
	if (lpelf->lfFaceName[0] != TEXT('@'))
	{
		SendMessage(*(HWND*)lParam, LB_ADDSTRING, 0, (LPARAM)lpelf->lfFaceName);
	}
	return 1;
}

int CALLBACK EnumFontFamiliesExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam)
{
	CopyMemory((void*)lParam, &lpelfe->elfLogFont, sizeof(LOGFONT));
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hFont;
	static HWND hTrack;
	static HWND hList;
	switch (msg)
	{
	case WM_CREATE:
		InitCommonControls();
		hFont = CreateFont(64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("メイリオ"));

		hTrack = CreateWindow(TRACKBAR_CLASS, 0, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE, 10, 300, 128, 32, hWnd, 0, NULL, NULL);
		SendMessage(hTrack, TBM_SETRANGE, TRUE, MAKELONG(16, 255));
		SendMessage(hTrack, TBM_SETPAGESIZE, 0, 20);
		SendMessage(hTrack, TBM_SETTICFREQ, 20, 0);
		SendMessage(hTrack, TBM_SETPOS, TRUE, 16);

		hList = CreateWindow(TEXT("LISTBOX"), 0, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_STANDARD, 10, 350, 256, 512, hWnd, (HMENU)100, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		{
			HDC hdc = GetDC(0);
			EnumFontFamilies(hdc, 0, EnumFontFamProc, (LPARAM)&hList);
			ReleaseDC(hWnd, 0);
			SendMessage(hList, LB_SETCURSEL, 0, 0);
		}

		break;
	case WM_HSCROLL:
		DeleteObject(hFont);
		hFont = CreateFont((int)SendMessage(hTrack, TBM_GETPOS, 0, 0), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("メイリオ"));
		InvalidateRect(hWnd, 0, 1);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == 100 && HIWORD(wParam) == LBN_SELCHANGE)
		{
			DWORD_PTR dwCursel = SendMessage(hList, LB_GETCURSEL, 0, 0);
			LPTSTR strText = (LPTSTR)GlobalAlloc(GMEM_FIXED, sizeof(TCHAR)*(SendMessage(hList, LB_GETTEXTLEN, dwCursel, 0) + 1));
			SendMessage(hList, LB_GETTEXT, dwCursel, (LPARAM)strText);
			LOGFONT lf;
			memset(&lf, 0, sizeof(LOGFONT));
			lf.lfCharSet = DEFAULT_CHARSET;
			lstrcpy(lf.lfFaceName, strText);
			GlobalFree(strText);
			HDC hdc = GetDC(0);
			EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamiliesExProc, (LPARAM)&lf, 0);
			ReleaseDC(0, hdc);
			DeleteObject(hFont);
			hFont = CreateFontIndirect(&lf);
			InvalidateRect(hWnd, 0, 1);
		}
		return 0;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			LPCTSTR lpszText = TEXT("こんにちは");
			{
				HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

				TextOut(hdc, 10, 10, lpszText, lstrlen(lpszText));

				SelectObject(hdc, hOldFont);
			}

			{
				Gdiplus::Graphics g(hdc);
				RectF out;
				g.MeasureString(L" ", 1, &Font(hdc, hFont), PointF(0,0), &out);				
				g.DrawString(lpszText, -1, &Font(hdc, hFont), PointF(10 - out.Width / 2.0f, 50), &SolidBrush(Color::Black));
			}

			RECT rect;
			GetClientRect(hWnd, &rect);

			MoveToEx(hdc, 10, 0, 0);
			LineTo(hdc, 10, rect.bottom);

			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		DeleteObject(hFont);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	ULONG_PTR gdiToken;
	GdiplusStartupInput gdiSI;
	GdiplusStartup(&gdiToken, &gdiSI, NULL);

	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gdiToken);

	return (int)msg.wParam;
}
