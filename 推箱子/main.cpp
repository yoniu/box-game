#include <windows.h>
#include <ctime>
#include <cstdlib>

#define ROW 20
#define COL 10
#define GRID 50
#define WIDTH ROW * GRID
#define HEIGHT COL * GRID
#define BOXLENGTH 3

struct Position {
	int x;
	int y;
};
struct PositionEx { // 箱子位置
	int x;
	int y;
	bool move; // 是否可以移动
};
enum BlockType { // 板块类型
	Empty,
	Box,
	Aim,
	Stone
};

Position GetRandomPosition();
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
	static Position my, aimPosition[BOXLENGTH], stonePosition[BOXLENGTH];
	static PositionEx boxPosition[BOXLENGTH];
	static BlockType blockType[ROW][COL];
	static int currentBox, success;
	static HPEN hPen;
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
			// 随机数
			srand(time(0));
			// 创建目标位置
			success = 0;
			hPen = CreatePen(PS_SOLID, 3, RGB(0, 229, 0));
			SelectObject(hdcMem, hPen);
			for (int i = 0; i < BOXLENGTH; i++) 
			{
				aimPosition[i] = GetRandomPosition();
				blockType[aimPosition[i].x][aimPosition[i].y] = Aim;
				Rectangle(hdcMem, aimPosition[i].x * GRID, aimPosition[i].y * GRID, aimPosition[i].x * GRID + GRID, aimPosition[i].y * GRID + GRID);
			}
			DeleteObject(hPen);
			// 创建障碍物位置
			HDC hdcStone = CreateCompatibleDC(hdc);
			HBITMAP stone = (HBITMAP)LoadImage(NULL, L"stone.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			SelectObject(hdcStone, stone);
			DeleteObject(stone);
			for (int i = 0; i < BOXLENGTH; i++)
			{
				stonePosition[i] = GetRandomPosition();
				blockType[stonePosition[i].x][stonePosition[i].y] = Stone;
				BitBlt(hdcMem, stonePosition[i].x * GRID, stonePosition[i].y * GRID, GRID, GRID, hdcStone, 0, 0, SRCCOPY);
			}
			DeleteObject(hdcStone);
			// 创建到达目标画笔颜色
			hPen = CreatePen(PS_SOLID, 3, RGB(229, 0, 0));
			SelectObject(hdcMem, hPen);
			// 初始化箱子位置
			currentBox = 0;
			for (int i = 0; i < BOXLENGTH; i++) 
			{
				Position temp = GetRandomPosition();
				boxPosition[i].x = temp.x;
				boxPosition[i].y = temp.y;
				boxPosition[i].move = true;
				blockType[boxPosition[i].x][boxPosition[i].y] = Box;
			}
			ReleaseDC(hWnd, hdc);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		FillRect(hdc, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
		// 载入双缓冲
		BitBlt(hdc, my.x, my.y, WIDTH, HEIGHT, hdcMem, 0, 0, SRCCOPY);
		// 绘制箱子
		for(int i = 0; i < BOXLENGTH; i++)
			BitBlt(hdc, my.x + boxPosition[i].x * GRID, my.y + boxPosition[i].y * GRID, GRID, GRID, hdcBox, 0, 0, currentBox == i ? BLACKNESS : SRCCOPY); // BLACKNESS
		// 游戏成功
		if (success == BOXLENGTH)
			DrawText(hdc, L"游戏成功", -1, &rt, DT_CENTER);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) return (SendMessageW(hWnd, WM_DESTROY, NULL, NULL), 0);
		blockType[boxPosition[currentBox].x][boxPosition[currentBox].y] = Empty;
		switch (wParam)
		{
			case VK_UP:
			{
				if (boxPosition[currentBox].y != 0 && boxPosition[currentBox].move) {
					if (blockType[boxPosition[currentBox].x][boxPosition[currentBox].y - 1] == Empty)
						boxPosition[currentBox].y -= 1;
					else if (blockType[boxPosition[currentBox].x][boxPosition[currentBox].y - 1] == Aim) {
						success++;
						boxPosition[currentBox].y -= 1;
						boxPosition[currentBox].move = false;
						Rectangle(hdcMem, boxPosition[currentBox].x * GRID, boxPosition[currentBox].y * GRID, boxPosition[currentBox].x * GRID + GRID, boxPosition[currentBox].y * GRID + GRID);
					}
				}
			}
			break;
			case VK_DOWN:
			{
				if (boxPosition[currentBox].y < COL - 1 && boxPosition[currentBox].move) {
					if (blockType[boxPosition[currentBox].x][boxPosition[currentBox].y + 1] == Empty)
						boxPosition[currentBox].y += 1;
					else if (blockType[boxPosition[currentBox].x][boxPosition[currentBox].y + 1] == Aim) {
						success++;
						boxPosition[currentBox].y += 1;
						boxPosition[currentBox].move = false;
						Rectangle(hdcMem, boxPosition[currentBox].x * GRID, boxPosition[currentBox].y * GRID, boxPosition[currentBox].x * GRID + GRID, boxPosition[currentBox].y * GRID + GRID);
					}
				}
			}
			break;
			case VK_LEFT:
			{
				if (boxPosition[currentBox].x != 0 && boxPosition[currentBox].move) {
					if (blockType[boxPosition[currentBox].x - 1][boxPosition[currentBox].y] == Empty)
						boxPosition[currentBox].x -= 1;
					else if (blockType[boxPosition[currentBox].x - 1][boxPosition[currentBox].y] == Aim) {
						success++;
						boxPosition[currentBox].x -= 1;
						boxPosition[currentBox].move = false;
						Rectangle(hdcMem, boxPosition[currentBox].x * GRID, boxPosition[currentBox].y * GRID, boxPosition[currentBox].x * GRID + GRID, boxPosition[currentBox].y * GRID + GRID);
					}
				}
			}
			break;
			case VK_RIGHT:
			{
				if (boxPosition[currentBox].x < ROW - 1 && boxPosition[currentBox].move) {
					if (blockType[boxPosition[currentBox].x + 1][boxPosition[currentBox].y] == Empty)
						boxPosition[currentBox].x += 1;
					else if (blockType[boxPosition[currentBox].x + 1][boxPosition[currentBox].y] == Aim) {
						success++;
						boxPosition[currentBox].x += 1;
						boxPosition[currentBox].move = false;
						Rectangle(hdcMem, boxPosition[currentBox].x * GRID, boxPosition[currentBox].y * GRID, boxPosition[currentBox].x * GRID + GRID, boxPosition[currentBox].y * GRID + GRID);
					}
				}
			}
			break;
		}
		blockType[boxPosition[currentBox].x][boxPosition[currentBox].y] = Box;
		InvalidateRect(hWnd, NULL, true);
		return 0;
	case WM_LBUTTONDOWN:
		{
			int x = (LOWORD(lParam) - my.x) / 50,
				y = (HIWORD(lParam) - my.y) / 50;
			if (x > ROW - 1 || y > COL - 1 || x < 0 || y < 0) return 0;
			/*wchar_t b[10];
			wsprintf(b, L"%d, %d, %d", blockType[x][y], x, y);
			MessageBox(NULL, b, L"MessageBox弹出整型数据", 0);*/
			for(int i = 0; i < BOXLENGTH; i++)
				if (x == boxPosition[i].x && y == boxPosition[i].y)
				{
					currentBox = i;
					InvalidateRect(hWnd, NULL, true);
					break;
				}
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
		DeleteObject(hPen);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
Position GetRandomPosition() 
{
	Position m;
	m.x = rand() % (ROW - 0) + 0;
	m.y = rand() % (COL - 0) + 0;
	return m;
}