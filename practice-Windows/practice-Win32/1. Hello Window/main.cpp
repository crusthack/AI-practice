#include <Windows.h>


#define IDC_BUTTON 1001
#define IDC_EDIT   1002
#define IDC_LABEL  1003

HRESULT InitWindow(HINSTANCE hInstance, int nCmdSHow);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddButton();

HWND hwnd;

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdSHow
	)
{
	if (FAILED(InitWindow(hInstance, nCmdSHow)))
		return 0;

	MSG msg = {};
	while (GetMessage(&msg, hwnd, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return 0;
}

HRESULT InitWindow(HINSTANCE hInstance,	int nCmdShow)
{
	LPCWSTR className = L"Sample Window Class";

	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.lpfnWndProc = WindowProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = className;

	if (!RegisterClassExW(&wcex))
		return E_FAIL;

	hwnd = CreateWindowExW(WS_EX_LAYERED, className, L"Learn to Program Windows", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
		NULL, NULL, hInstance, NULL);

	if (!hwnd)
		return E_FAIL;

	SetLayeredWindowAttributes(hwnd, 0, 240, LWA_ALPHA);

	ShowWindow(hwnd, nCmdShow);

	CreateWindowExW(
		0,
		L"BUTTON",
		L"Click",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		20, 20, 100, 30,
		hwnd,
		(HMENU)1,
		hInstance,
		NULL
	);

	CreateWindowW(
		L"STATIC",
		L"Name:",
		WS_CHILD | WS_VISIBLE,
		20, 70, 80, 25,
		hwnd,
		(HMENU)IDC_LABEL,
		hInstance,
		NULL
	);

	CreateWindowW(
		L"EDIT",
		L"",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		100, 70, 200, 25,
		hwnd,
		(HMENU)IDC_EDIT,
		hInstance,
		NULL
	);

	CreateWindowW(
		L"BUTTON",
		L"확인",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		20, 110, 100, 30,
		hwnd,
		(HMENU)IDC_BUTTON,
		hInstance,
		NULL
	);

	return S_OK;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ret;
	switch (uMsg)
	{
	case WM_CLOSE:
		ret = MessageBox(hwnd, L"Really quit?", L"My application", MB_OKCANCEL);
		switch (ret)
		{
		case IDOK:
			DestroyWindow(hwnd);
			return 0;
		case IDCANCEL:
			MessageBoxW(hwnd, L"Cancel clicked", L"message", MB_ABORTRETRYIGNORE);
			return 0;
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == 1)
		{
			MessageBoxW(hwnd, L"Button clicked", L"Info", MB_OK);
		}
		if (LOWORD(wParam) == IDC_BUTTON)
		{
			wchar_t buffer[256]{};

			HWND edit = GetDlgItem(hwnd, IDC_EDIT);
			GetWindowTextW(edit, buffer, 256);

			MessageBoxW(hwnd, buffer, L"입력한 텍스트", MB_OK);
			AddButton();
		}
		return 0;
	}

	return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void AddButton()
{
	static int x = 50;

	CreateWindowW(
		L"BUTTON",
		L"확인",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, 110, 100, 30,
		hwnd,
		(HMENU)IDC_BUTTON,
		NULL,
		NULL
	);
	x += 10;
}