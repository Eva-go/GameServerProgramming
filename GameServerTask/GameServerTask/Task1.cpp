#include <iostream>
#include <windows.h>
#include <conio.h>
#pragma warning(disable:4996)
using namespace std;

#define MAPSIZE 800
#define MAPLIENNUM 8

HINSTANCE g_hInst;
LPCTSTR lpszClass = "Window Class Name";
LPCTSTR lpszWindowName = "Window Programming Lab";
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HDC hdc;
PAINTSTRUCT ps;
HBRUSH Brush, oBrush;

int mapLine[] = { 0,100,200,300,400,500,600,700,800 };
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;
	// 윈도우 클래스 구조체 값 설정
	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	// 윈도우 클래스 등록
	RegisterClassEx(&WndClass);
	// 윈도우 생성
	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, MAPSIZE+20, MAPSIZE+40, NULL, (HMENU)NULL, hInstance, NULL);
	// 윈도우 출력
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	// 이벤트 루프 처리
	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 메시지 처리하기
	static int X = 100, Y = 100;
	
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CREATE:
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		//MAP_Line
		for (int i = 0; i < MAPLIENNUM; i++) {
			MoveToEx(hdc, i * 100, 0, NULL);
			LineTo(hdc, i*100, MAPLIENNUM*100 - 1);
		}
		for (int i = 0; i < MAPLIENNUM; i++) {
			MoveToEx(hdc, 0, i * 100, NULL);
			LineTo(hdc, MAPLIENNUM * 100 - 1, i * 100);
		}
		Brush = CreateSolidBrush(RGB(255, 0, 0));
		oBrush = (HBRUSH)SelectObject(hdc, Brush);
		Rectangle(hdc, X, Y, X+100, Y-100);
		SelectObject(hdc, oBrush);
		DeleteObject(Brush);
		EndPaint(hWnd, &ps);
		break;
	case WM_KEYDOWN:
		{
			switch (wParam)
			{
			case VK_LEFT:
				X -= 100;
				break;
			case VK_RIGHT:
				X += 100;
				break;
			case VK_UP:
				Y -= 100;
				break;
			case VK_DOWN:
				Y += 100;
				break;
			}
		}
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam); // 위의 세 메시지 외의 나머지 메시지는 OS로
}
