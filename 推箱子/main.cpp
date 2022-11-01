#include <windows.h>
#define ROW 20
#define COL 10
#define GRID 50
#define WIDTH ROW * GRID
#define HEIGHT COL * GRID

struct Position {
	int x;
	int y;
};
enum BlockType {
	Empty,
	Box,
	Aim
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPreInst,
	LPSTR lpszCmdLine,
	int nCmdShow
)
{
	WNDCLASS wc = { 0 };
	const wchar_t* WndTitle = L"许景智 2220631128";
	const wchar_t* WndClassName = L"GAMEAPP";

	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WndClassName;
	wc.hbrBackground = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.lpszMenuName = nullptr;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClass(&wc))
	{
		return MessageBox(NULL, L"注册窗口失败", L"提示：", MB_OK);
	}


	HWND hWnd;
	hWnd = CreateWindow(
		WndClassName,
		WndTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	ShowWindow(hWnd, SW_MAXIMIZE);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static HDC hdcMem, hdcBox;
	static HBITMAP hbm;
	static RECT rt;
	static Position my, boxPosition;
	static BlockType blockType[ROW][COL];
	switch (message)
	{
	case WM_CREATE:
		{
			// 计算放置网格位置
			GetClientRect(hWnd, &rt);
			my.x = (rt.right - rt.left) / 2 - WIDTH / 2;
			my.y = (rt.bottom - rt.top) / 2 - HEIGHT / 2;
			// 创建双缓冲
			hdc = GetDC(hWnd);
			hdcMem = CreateCompatibleDC(hdc);
			hbm = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
			SelectObject(hdcMem, hbm);
			SelectObject(hdcMem, GetStockObject(WHITE_BRUSH));
			Rectangle(hdcMem, 0, 0, WIDTH, HEIGHT);
			// 绘制网格
			for (int i = 0; i <= ROW; i++) {
				MoveToEx(hdcMem, i * GRID, 0, NULL);
				LineTo(hdcMem, i * GRID, GRID * COL);
			}
			for (int j = 0; j <= COL; j++) {
				MoveToEx(hdcMem, 0, j * GRID, NULL);
				LineTo(hdcMem, GRID * ROW, j * GRID);
			}
			// 初始化网格位置信息
			for (int i = 0; i < COL; i++)
				for (int j = 0; j < ROW; j++)
					blockType[j][i] = Empty;
			// 箱子图片
			hdcBox = CreateCompatibleDC(hdc);
			HBITMAP box = (HBITMAP)LoadImage(NULL, L"box1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			SelectObject(hdcBox, box);
			DeleteObject(box);
			// 初始化箱子位置
			boxPosition = { 0 , 0 };
			blockType[boxPosition.x][boxPosition.y] = Box;
			ReleaseDC(hWnd, hdc);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		FillRect(hdc, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
		BitBlt(hdc, my.x, my.y, WIDTH, HEIGHT, hdcMem, 0, 0, SRCCOPY);
		BitBlt(hdc, my.x + boxPosition.x * GRID, my.y + boxPosition.y * GRID, GRID, GRID, hdcBox, 0, 0, SRCCOPY); // BLACKNESS
		EndPaint(hWnd, &ps);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			return (SendMessageW(hWnd, WM_DESTROY, NULL, NULL), 0);
		}
		else
		{
			blockType[boxPosition.x][boxPosition.y] = Empty;
			if (wParam == VK_UP) {
				boxPosition.y -= (boxPosition.y == 0) ? 0 : 1;
			}
			else if (wParam == VK_DOWN) {
				boxPosition.y += (boxPosition.y >= COL - 1) ? 0 : 1;
			}
			else if (wParam == VK_LEFT) {
				boxPosition.x -= (boxPosition.x == 0) ? 0 : 1;
			}
			else if (wParam == VK_RIGHT) {
				boxPosition.x += (boxPosition.x >= ROW - 1) ? 0 : 1;
			}
		}
		blockType[boxPosition.x][boxPosition.y] = Box;
		InvalidateRect(hWnd, NULL, true);
		return 0;
	case WM_LBUTTONDOWN:
		{
			int x = (LOWORD(lParam) - my.x) / 50,
				y = (HIWORD(lParam) - my.y) / 50;
			if (x > ROW - 1 || y > COL - 1 || x < 0 || y < 0) return 0;
			wchar_t b[10];
			wsprintf(b, L"%d, %d, %d", blockType[x][y], x, y);
			MessageBox(NULL, b, L"MessageBox弹出整型数据", 0);
		}
		return 0;
	case WM_SIZE:
		// 窗口大小改变重新计算网格放置位置
		GetClientRect(hWnd, &rt);
		my.x = (rt.right - rt.left) / 2 - WIDTH / 2;
		my.y = (rt.bottom - rt.top) / 2 - HEIGHT / 2;
		InvalidateRect(hWnd, NULL, true);
		return 0;
	case WM_DESTROY:
		DeleteObject(hdcMem);
		DeleteObject(hdcBox);
		DeleteObject(hbm);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}