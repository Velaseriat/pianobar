/*
Copyright (c) 2008-2014
	Lars-Dominik Braun <lars@6xq.net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "config.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>

#if defined(BAR_WINDOWS)
#include <windows.h>

static DWORD restoreMode;
static bool restoreModeValid = false;

void BarTermInit () {
	HANDLE const stdinHandle = GetStdHandle (STD_INPUT_HANDLE);
	DWORD currentMode;

	if (stdinHandle == INVALID_HANDLE_VALUE ||
			!GetConsoleMode (stdinHandle, &currentMode)) {
		return;
	}

	restoreMode = currentMode;
	restoreModeValid = true;

	currentMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
	SetConsoleMode (stdinHandle, currentMode);
}

void BarTermRestore () {
	if (!restoreModeValid) {
		return;
	}

	HANDLE const stdinHandle = GetStdHandle (STD_INPUT_HANDLE);
	if (stdinHandle != INVALID_HANDLE_VALUE) {
		SetConsoleMode (stdinHandle, restoreMode);
	}
}
#else
#include <termios.h>
#include <unistd.h>

/* need a global variable here, since these functions get called from a signal
 * handler */
static struct termios restore;

/*	init terminal attributes when continuing, assuming the shell modified them;
 *	tcget/setattr and signal are async signal safe */
static void BarTermHandleCont (int sig) {
	(void) sig;
	BarTermInit ();
}

void BarTermInit () {
	struct termios newopt;

	tcgetattr (STDIN_FILENO, &restore);
	memcpy (&newopt, &restore, sizeof (newopt));

	/* disable echoing and line buffer */
	newopt.c_lflag &= ~(ECHO | ICANON);
	tcsetattr (STDIN_FILENO, TCSANOW, &newopt);

	signal (SIGCONT, BarTermHandleCont);
}

void BarTermRestore () {
	tcsetattr (STDIN_FILENO, TCSANOW, &restore);
}
#endif
