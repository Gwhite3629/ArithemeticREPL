#include "../MathREPL/winhelp.h"
#include "syntax.h"
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <WinUser.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <conio.h>
#include <process.h>

void print_screen(map_t* map, char* line)
{
	return;
}

inline State_t* GetWinState(HWND hwnd)
{
	LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	State_t* state = (State_t*)ptr;
	return state;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg) {
		case WM_KEYDOWN:
			switch (wParam) {
			case VK_UP:
				if (state->hindex == (state->hmax - 1))
					break;
				state->hindex++;
				sprintf(state->userin, "%s", state->history[state->hindex]);
				state->index = (unsigned int)strlen(state->history[state->hindex]);
				break;
			case VK_DOWN:
				if (state->hindex == 0) {
					memset(state->userin, '\0', 80);
					state->index = 0;
					break;
				}
				state->hindex--;
				sprintf(state->userin, "%s", state->history[state->hindex]);
				state->index = (unsigned int)strlen(state->history[state->hindex]);
				break;
			case VK_LEFT:
				if (state->index == 0)
					break;
				state->index--;
				sprintf(state->history[0], "%s", state->userin);
				break;
			case VK_RIGHT:
				if (state->index == strlen(state->userin))
					break;
				state->index++;
				sprintf(state->history[0], "%s", state->userin);
				break;
			case VK_RETURN:
				if (strlen(state->userin) > 0) {
					print_screen(&state->map, state->userin);
					state->hindex = 0;
					for (unsigned int i = state->hmax; i > 0; i--) {
						sprintf(state->history[i], "%s", state->history[i - 1]);
					}
					state->hmax++;
					run(state);
					memset(state->userin, '\0', 80);
					state->index = 0;
				}
				break;
			case VK_BACK:
				if (state->index == 0)
					break;
				else if (state->index == strlen(state->userin)) {
					state->userin[state->index - 1] = '\0';
				}
				else {
					memcpy(state->tmp1, state->userin, state->index - 1);
					memcpy(state->tmp2, state->userin + state->index, strlen(state->userin) - state->index);
					memset(state->userin, '\0', 80);
					sprintf(state->userin, "%s%s", state->tmp1, state->tmp2);
					memset(state->tmp1, '\0', 80);
					memset(state->tmp2, '\0', 80);
				}
				state->index--;
				sprintf(state->history[0], "%s", state->userin);
				break;
			}
			sprintf(state->history[0], "%s", state->userin);
			OnPaint();
			break;
		case WM_CHAR:
			if (wParam > 31) {
				if (strlen(state->userin) == 80)
					break;
				else if (state->index == strlen(state->userin))
					state->userin[state->index] = (char)wParam;
				else {
					memcpy(state->tmp1, state->userin, state->index);
					memcpy(state->tmp2, state->userin + state->index, strlen(state->userin) - state->index);
					sprintf(state->userin, "%s%c%s", state->tmp1, (char)wParam, state->tmp2);
					memset(state->tmp1, '\0', 80);
					memset(state->tmp2, '\0', 80);
				}
				state->index++;
				sprintf(state->history[0], "%s", state->userin);
				OnPaint();
				
			}
			break;
		case WM_CLOSE:
			state->q = 1;
			DestroyWindow(m_hwnd);
			break;
		case WM_SIZE:;
			Resize();
			break;
		case WM_PAINT:;
			OnPaint();
			break;
		case WM_CREATE:
			if (FAILED(D2D1CreateFactory(
				D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
			{
				return -1;  // Fail CreateWindowEx.
			}
			if (FAILED(DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&pDWriteFactory)
			)))
			{
				return -1;
			}
			break;
		default:
			return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}
	return 0;
}