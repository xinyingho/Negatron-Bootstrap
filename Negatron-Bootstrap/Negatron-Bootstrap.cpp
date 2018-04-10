/*
 * This file is part of Negatron.
 * Copyright (C) 2015-2016 BabelSoft S.A.S.U.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
// Negatron-Bootstrap.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Negatron-Bootstrap.h"

#pragma comment(lib, "Shlwapi.lib")

// Forward declarations of functions included in this code module:
TCHAR * concat(TCHAR* format, ...);

int APIENTRY _tWinMain(
	HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow
) {
	// Determine current working directory
	TCHAR workingPath[MAX_PATH];
    if(!GetModuleFileName(NULL, workingPath, MAX_PATH))
        return GetLastError();
	TCHAR * endPath = _tcsrchr(workingPath, TEXT('\\'));
	*endPath = '\0';

	// Look for any portable versions of Java included in Negatron's installation folder
	WIN32_FIND_DATA fileData;
	TCHAR jrePath[MAX_PATH] = TEXT("javaw.exe");
	TCHAR jrePathMask[MAX_PATH];
	_tcscpy_s(jrePathMask, workingPath);
	_tcscat_s(jrePathMask, TEXT("\\jre*"));
	HANDLE hFind = FindFirstFile(jrePathMask, &fileData);
	if (hFind != INVALID_HANDLE_VALUE) do {
		if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			_tcscpy_s(jrePathMask, fileData.cFileName);
			_tcscat_s(jrePathMask, TEXT("\\bin\\javaw.exe"));
			if (PathFileExists(jrePathMask) == TRUE) {
				_tcscpy_s(jrePath, jrePathMask);
				break;
			}
		}
	} while (FindNextFile(hFind, &fileData) != 0);
	if (hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);

	// Launch Negatron
	TCHAR * arguments;
	if (*lpCmdLine == '\0')
		arguments = concat(TEXT("%s -Xms512m -Xmx2g -jar \"%s\\Negatron.jar\""), jrePath, workingPath);
	else
		arguments = concat(TEXT("%s %s -jar \"%s\\Negatron.jar\""), jrePath, lpCmdLine, workingPath);

	//ShellExecute(NULL, NULL, jrePath, arguments, workingPath, SW_SHOWNORMAL);
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	if(!CreateProcess(NULL,   // No module name (use command line)
		arguments,      // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure
    )) {
        printf("CreateProcess failed (%d).\n", GetLastError());
		free(arguments);
        return 0;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
	free(arguments);

	return 0;
}

TCHAR * concat(TCHAR* format, ...) {
   va_list args;
   int len;
   TCHAR * buffer;

   va_start(args, format);
   len = _vsctprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
   buffer = (TCHAR *) malloc(len * sizeof(TCHAR));
   _vstprintf_s(buffer, len, format, args);

   return buffer;
}
