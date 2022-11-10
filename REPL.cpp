#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "utils.h"
#include "REPLutils.h"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	unsigned int ret = SUCCESS;
	MSG msg = { };

	MainWindow win;

	State_t* state = NULL;
	MEM(state, 1, State_t);
	create_map(&state->map);
	state->q = 0;
	state->hmax = 1;
	state->hindex = 0;
	state->index = 0;

	win.give_state(state);

	if (!win.Create(L"Arithmetic REPL", WS_OVERLAPPEDWINDOW))
	{
		return 0;
	}

	ShowWindow(win.Window(), nCmdShow);

	// Run the message loop.

	while (GetMessage(&msg, win.get_win(), 0, 0) && !state->q)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

exit:
	if (state) {
		destroy_map(&state->map);
		free(state);
		state = NULL;
	}

	return ret;
}