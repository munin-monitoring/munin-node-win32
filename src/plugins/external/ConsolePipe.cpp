
#include "stdafx.h"
#include "ConsolePipe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CConsolePipe::CConsolePipe(DWORD flags) : m_bIsNT((LONG)GetVersion() >= 0), m_dwFlags(flags)
{
	m_hListenerThread = NULL;
	m_hChildProcess = NULL;
	m_hInputWrite = INVALID_HANDLE_VALUE;
	m_hOutputRead = INVALID_HANDLE_VALUE;
}

CConsolePipe::~CConsolePipe() {
	Cleanup();
}

// stop the background process and cleanup
// DON'T USE THIS METHOD UNLESS YOU HAVE EXHAUSTED ALL OTHER POSSIBLE EXIT ROUTES
// this is of limited value since main app can't tell if this class has already self-destructed!
// if a cached pointer of this class is used, better use IsBadReadPtr to arse-cover yourself
void CConsolePipe::Break() {
	assert( // assume normal "new"
		//_CrtIsMemoryBlock(this, sizeof(CConsolePipe), 0,0,0) &&
		IsBadReadPtr(this, sizeof(CConsolePipe))==0);
	// unfortunately _CrtIsMemoryBlock don't exists in release builds so we can't protect here

	if(IsChildRunning()) {
		assert( !(CPF_REUSECMDPROC & m_dwFlags) ); // try a StopCmd() first!
		assert(m_hChildProcess);
		assert(m_bIsNT && _T("If you proceed further in win9x you'll crash windows! (likely)"));

		// acting like a bully again
		TerminateProcess(m_hChildProcess, 10);

		// on platforms tested, the killed process doesn't cleanup the pipes
		// i have to do a forced cleanup myself

		// an idea to terminate the listener thread cleanly would be CloseHandle(m_hOutputRead)
		// unfortunately it doesn't work -- in fact it locks the program altogether! (must be
		// because we're attempting closing a handle we're waiting (ReadFile) on)
	}

	// in all (some) cases the object self-destructs after Break()
	if( CPF_NOAUTODELETE & m_dwFlags)
		Cleanup(); // instead of deletion, prepare class for reuse
	else
		delete this;
}

// call this when your (GUI) application exits to cleanup any forced consoles
void CConsolePipe::Term() {
	if(m_bForcedConsole) {
		/* WARNING %%%
			* Q197630 describes a glitch whereby processes that call AllocConsole() don't receive
			* WM_ENDSESSION, but perhaps that's for NT only. If you really want this message then
			* SetConsoleCtrlHandler offers an alternative.
			*/

#ifdef _DEBUG
		assert(IsConsoleAttached());
		ATLTRACE(_T("Releasing forced console...\n"));
#endif

		FreeConsole(); // ensures console destruction
		// i have seen situations in XP where console ain't destroyed when main app exits

		m_bForcedConsole = FALSE;
	}
}

// at most one console is attached per process, this tests whether this is the case
BOOL CConsolePipe::IsConsoleAttached() {
	// for NT, invalid handles are indicated by a NULL return
	// for 9x on the other hand we get INVALID_HANDLE_VALUE
	return (long)GetStdHandle(STD_ERROR_HANDLE) > 0;
}

// execute a command with a DOS command processor; see CPEXEC_xxx for return values
int CConsolePipe::Execute(LPCTSTR pszCommand) {
	assert(pszCommand && *pszCommand);

	// for (running) reusable command processors, send this as another input
	if(m_hListenerThread) {
		if(CPF_REUSECMDPROC & m_dwFlags) {
			TString strCmd(pszCommand);
            assert(strCmd.find(_T('\n')) == TString::npos);
			strCmd += _T("\r\n");

			/* WINDOWS APPLICATIONS
				* If one tries to launch a windows proggy (e.g. notepad) most of the time nowt
				* happens first time round. If a second newline is keyed then the window appears.
				* I have checked with the task manager and sure enough the process exist from the
				* first time, only it is invisible! I haven't got a clue why this happens; perhaps
				* it's the SW_HIDE passed in the original CreateProcess, but that is required @@@
				*/
			SendChildInput(strCmd.c_str());
			return CPEXEC_OK; // can't tell if it is busy...
			// moreover, if cmd is busy with a proggy that accepts input, our command will be lost
		}
	}

	assert(NULL == m_hChildProcess);
	assert(INVALID_HANDLE_VALUE == m_hInputWrite);
	assert(INVALID_HANDLE_VALUE == m_hOutputRead);
	if(m_hListenerThread)
		return CPEXEC_MISC_ERROR;
	if(pszCommand==0 || *pszCommand == 0)
		return CPEXEC_COMMAND_ERROR;

	// for 9x we need a console for THIS process to be inherited by the child
	// this circumvents the ugly stub processes recommended by Q150956
	// ADDENDUM: ok for NT too in the end, since I want to be able to send Ctrl-C events
	if(!IsConsoleAttached()) {
		if(!SetupConsole()) {
#ifdef _DEBUG
			ATLTRACE(_T("MINI-assert: can't create console window\n"));
#endif
			return CPEXEC_MISC_ERROR;
		}
	}

	// execution with redirection follows guidelines from:
	// Q190351 - HOWTO: Spawn Console Processes with Redirected Standard Handles

	HANDLE loadzaHandles[5]; // for EZ cleanup
    int i;
	for(i = 0; i < sizeof(loadzaHandles)/sizeof(loadzaHandles[0]); i++)
		loadzaHandles[i] = INVALID_HANDLE_VALUE;
#if 0 // that's the array correspondence, in order
    HANDLE hOutputReadTmp, hOutputWrite;
    HANDLE hInputWriteTmp, hInputRead;
    HANDLE hErrorWrite;
#endif
    SECURITY_ATTRIBUTES sa;

    // Set up the security attributes struct.
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

	int status;
	HANDLE hProc = GetCurrentProcess();
    // Create the child output pipe, default buffer size
	if(CreatePipe(&loadzaHandles[0], &loadzaHandles[1], &sa, 0) &&

    // Create a duplicate of the output write handle for the std error
    // write handle. This is necessary in case the child application
    // closes one of its std output handles. (you don't say! :)
		DuplicateHandle(hProc, loadzaHandles[1],
                        hProc, &loadzaHandles[4], 0,
                        TRUE, DUPLICATE_SAME_ACCESS) &&

	// Create the child input pipe.
		CreatePipe(&loadzaHandles[3], &loadzaHandles[2], &sa, 0) &&

	// Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
		DuplicateHandle(hProc, loadzaHandles[0],
                        hProc,
                        &m_hOutputRead, // Address of new handle.
                        0, FALSE, // Make it uninheritable.
                        DUPLICATE_SAME_ACCESS) &&

		DuplicateHandle(hProc, loadzaHandles[2],
                        hProc,
                        &m_hInputWrite, // Address of new handle.
                        0, FALSE, // Make it uninheritable.
                        DUPLICATE_SAME_ACCESS))
	{
		// Close inheritable copies of the handles you do not want to be inherited.
		CloseHandle(loadzaHandles[0]);  loadzaHandles[0] = INVALID_HANDLE_VALUE;
		CloseHandle(loadzaHandles[2]);  loadzaHandles[2] = INVALID_HANDLE_VALUE;

		if(LaunchRedirected(pszCommand, loadzaHandles[1], loadzaHandles[3], loadzaHandles[4]))
		{
			// Close pipe handles (do not continue to modify the parent).
			// You need to make sure that no handles to the write end of the
			// output pipe are maintained in this process or else the pipe will
			// not close when the child process exits and the ReadFile will hang.
			for(i = 0; i < sizeof(loadzaHandles)/sizeof(loadzaHandles[0]); i++) {
				if(loadzaHandles[i] != INVALID_HANDLE_VALUE) {
					CloseHandle(loadzaHandles[i]);
					loadzaHandles[i] = INVALID_HANDLE_VALUE;
				}
			}
			// now all's left are the handles to the pipe ends we use for communication
			// i.e. m_hOutputRead (receive stdout) & m_hInputWrite (specify stdin)

			// create thread that monitors for printfs
			// since it don't uses CRT, a plain creation is sufficient
			DWORD ThreadId;
			m_hListenerThread = ::CreateThread(NULL, 0, ListenerThreadProc,
				(LPVOID)this, 0, &ThreadId);
			if(m_hListenerThread)
				status = CPEXEC_OK;
			else
				status = CPEXEC_MISC_ERROR; // rare
		}
		else
			status = CPEXEC_COMMAND_ERROR;
	}
	else
		status = CPEXEC_MISC_ERROR;

	// cleanup in case of errors
	for(i = 0; i < sizeof(loadzaHandles)/sizeof(loadzaHandles[0]); i++) {
		if(loadzaHandles[i] != INVALID_HANDLE_VALUE)
			CloseHandle(loadzaHandles[i]);
	}

	if(CPEXEC_OK != status) {
#ifdef _DEBUG
		ATLTRACE(_T("MINI-assert: can't launch redirected child (%d)\n"), status);
#endif
		assert(!m_hListenerThread);
		Cleanup();
		// now this object is ready for another Execute() attempt or destruction
	}

	// if this didn't succeed, clients should delete this object
	// for CPEXEC_MISC_ERROR try a non-posh alternative like CreateProcess/ShellExecute
	return status;
}

// write data to be read by child process from its stdin
// useful for commands that expect user input e.g. Y/N answers
BOOL CConsolePipe::SendChildInput(LPCTSTR pszText) {
	assert(m_hInputWrite != INVALID_HANDLE_VALUE);
	assert(IsChildRunning());
	assert(pszText);
	//assert(0 && _T("This method wasn't really tested, be careful!"));
#ifdef _DEBUG
	// usually sent commands come with a newline attached
	ATLTRACE(_T("PIPE %x: Sending command: %s "), this, pszText);
#endif

	/* UNICODE REVELATIONS %%%
		* In win2000 the /U command line option for cmd.exe is a (partial) fluke since it only
		* affects the output from commands. The stdin is still expected in ansi (!!) which I have
		* discovered after a long period of head-scratching. So this member should really be
		* accepting a LPCSTR argument, but the conversion is handled locally for convenience.
		* $TSEK the same happens for XP, didn't check for NT4
		*/

	DWORD dwLen = lstrlen(pszText);
	if(!dwLen)
		return FALSE;

	LPSTR oem;
#ifdef UNICODE
	// unlike when first created, we should do an explicit oem conversion here
	DWORD tmp = 2 * dwLen;
	oem = (LPSTR)_alloca(tmp + 1); // worst case for multibyte
	assert(oem && dwLen < 10000);
	tmp = WideCharToMultiByte( (m_dwFlags & CPF_NOCONVERTOEM) ? CP_ACP : CP_OEMCP, 
		0, pszText, dwLen, oem, tmp, NULL, NULL);
	assert(tmp == dwLen); // @@@ may break for multibyte?
	//oem[dwLen] = 0; no need
#else
	// i don't really get it but although the string has already the character encoded 
	// as e.g. "128", still the console cannot find it. I thought OEM was just a mapping thing!
	if(m_dwFlags & CPF_NOCONVERTOEM)
		oem = (LPSTR)pszText;
	else {
		// it must be down to ansi fileAPIs which have converted the name in the first place
		// @@@ still some special characters like the EURO get messed up in the process
		oem = (LPSTR)_alloca(dwLen + 1);
		assert(oem && dwLen < 10000);
		CharToOem(pszText, oem);
	}
#endif

	return SendChildInput(oem, dwLen);
}

BOOL CConsolePipe::SendChildInput(void *pData, DWORD dwSize) {
	assert(m_hInputWrite != INVALID_HANDLE_VALUE);
	assert(IsChildRunning());
	assert(pData);
	assert(dwSize > 0);

	DWORD dwWritten;
	return WriteFile(m_hInputWrite, pData, dwSize, &dwWritten, NULL);
}

// see comments in Break(); ensure that this instance ain't deleted first!
BOOL CConsolePipe::IsChildRunning() {
	if(m_hChildProcess) {
		if(m_bIsNT) {
			DWORD dwExitCode;
			if(GetExitCodeProcess(m_hChildProcess, &dwExitCode))
				return STILL_ACTIVE == dwExitCode;
		}
		else {
			// win9x GetExitCodeProcess is f@ked! it returns 0==exitcode even if running!
			// see Q111559; it is "by design" for 16-bit child processes like msdos
			return TRUE;
		}
	}
	return FALSE;
}

// for persistent command processors, send a command that will terminate cmd.exe
// typically this will be called before the main app exits
void CConsolePipe::StopCmd() {
	assert(CPF_REUSECMDPROC & m_dwFlags);
	assert( // assume normal "new"
		_CrtIsMemoryBlock(this, sizeof(CConsolePipe), 0,0,0) &&
		IsBadReadPtr(this, sizeof(CConsolePipe))==0);

	if(IsChildRunning()) {
		SendCtrlBrk(); // in case it was stuck

		if( (CPF_REUSECMDPROC & m_dwFlags) ) {
			//SendChildInput(^Z) would help stop some blocked processors
			Execute(_T("exit"));
			Sleep(200);
			// cleanup will occur "naturally"
		}
	}
}

// soft-break for processes like "more"
void CConsolePipe::SendCtrlBrk() {
	if(IsChildRunning()) {
		//GenerateConsoleCtrlEvent(CTRL_C_EVENT, m_dwProcessGroupId); // extra prodding for 9x
		// unfortunately, this has no effect for 9x, period. What crap OS this is!

		//TCHAR ctrlC[] = {3, 0};
		//SendChildInput(ctrlC); no effect, perhaps translated by pipework?

		// this is why we had to allocate a console even for NT
		if(GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, m_dwProcessGroupId))
			OnReceivedOutput(_T("<CtrlBreak>")); // visual reassurance
	}
}


BOOL CConsolePipe::SetupConsole() {
	assert(IsConsoleAttached()==0);
	assert(!m_bForcedConsole);

	/* CONSOLE VISIBILITY
		* Our console is just auxiliary and not meant to be visible. Alas I've searched high &
		* lo but couldn't find a way to persuade AllocConsole() to create an invisible console.
		* CBT_HOOKs don't work either. So it's down to lo-tech I'm afraid and there could be a
		* short flash in the screen for poor 9x users.
		*/

	// i don't cache this fn pointer since this is called just once
	HWND (WINAPI* gpGetConsoleWindow)() = (HWND (WINAPI*)())
		GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetConsoleWindow");

	LockWindowUpdate(GetDesktopWindow()); // sometimes it prevents flashing
	if(AllocConsole()) {
#ifdef _DEBUG
		ATLTRACE(_T("Created console for this process"));
		DWORD mi = 0, mo = 0;
		HANDLE hi = GetStdHandle(STD_INPUT_HANDLE), ho = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleMode(ho, &mo);
		GetConsoleMode(hi, &mi);
		ATLTRACE(_T(" (MODE I=%x, O=%x)\n"), mi, mo);
		UINT icp = GetConsoleCP(), ocp = GetConsoleOutputCP();
		ATLTRACE(_T("console CP: in = %d, out = %d\n"), icp, ocp);
		if( !(m_dwFlags & CPF_NOCONVERTOEM) )
			assert(icp == ocp && icp == GetOEMCP());
#endif

#if 0
		// the standard modes are not suitable for piping
		// but these changes have no effect, unfortunately, especially the echo
		mi &= ~(ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
		mo &= ~(ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
		SetConsoleMode(ho, 0/*mo*/);
		SetConsoleMode(hi, 0/*mi*/);
#endif

		m_bForcedConsole = TRUE;

		HWND wcon;
		if(gpGetConsoleWindow) {
			wcon = gpGetConsoleWindow();
			assert(wcon);
		}
		else {
			TString title = UGHGetUniqueName();

			// change current window title
			SetConsoleTitle(title.c_str());
			// ensure window title has been updated
			Sleep(40);

			/* CONSOLE WINDOW CLASS
				* For 9x it's called "tty"; for XP it's upgraded to "ConsoleWindowClass". All in all
				* there's no easy consistency hence I rely on the title and not on the class name.
				* BTW, if this was just for win2000+ then I could have used GetConsoleWindow API.
				*/
			wcon = FindWindow(NULL, title.c_str());
		}


		// If found, hide it
		if (wcon)
			ShowWindow(wcon, SW_HIDE);
#ifdef _DEBUG
		else 
			ATLTRACE(_T("MINI-assert: can't find console window\n"));
#endif
	}

	LockWindowUpdate(NULL);
	return m_bForcedConsole;

	/* ADDENDUM FOR WIN9x
		* Now that we have a hidden console, any attempts to launch a console app outside this
		* class may have problems. For example a CreateProcess call MUST have the dwCreationFlags
		* set to DETACHED_PROCESS (or CREATE_NEW_CONSOLE) so that it won't get trapped sharing 
		* a hidden console.
		*/
}

BOOL CConsolePipe::LaunchRedirected(LPCTSTR pszCommand, HANDLE hChildStdOut, HANDLE hChildStdIn, 
	HANDLE hChildStdErr)
{
	assert(hChildStdOut != INVALID_HANDLE_VALUE && 
		hChildStdIn != INVALID_HANDLE_VALUE &&
		hChildStdErr != INVALID_HANDLE_VALUE);
	assert(pszCommand); // command interpreter will be added on top
	assert(!m_hChildProcess);

    PROCESS_INFORMATION pi;
    STARTUPINFO si;

	/* REDIRECTION STRATEGY
		* In all cases we redirect std* handles of the child to the various pipe ends. Platform
		* differences concern the console; in 9x/Me we force the child to inherit our console; for
		* NTx we create a fresh hidden console.
		*/
    // Set up the start up info struct.
	memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hChildStdOut;
    si.hStdInput  = hChildStdIn;
    si.hStdError  = hChildStdErr;

	// cheeky thought that hasn't lead anywhere wrt echoing
	//SetConsoleMode(hChildStdIn, 0);
	//SetConsoleMode(hChildStdOut, 0);

	// another platform dependent detail is the command processor
	LPCTSTR sys; // i don't read %COMSPEC% since it is infrequently setup properly
	sys = m_bIsNT ? _T("cmd.exe /A") : _T("command.com");
	DWORD dwCreateFlags = CREATE_NEW_PROCESS_GROUP; // Ctrl-breakable

	/* ~UNICODE: PIPE DATA
		* cmd.exe has a couple of flags that determine whether the output is ansi/unicode (/A
		* & /U respectively). However the 9x command processor has no such equivalent, but then
		* again unicode is not supported there!
		*
		* REVISION. The /U flag has proven unreliable. Not only it is just for the output 
		* direction, there are situations where the kidz create additional command processors
		* (e.g. some batch files) and since _they_ don't use /U, our output gets mangled too.
		* So I go for ansi (OEM) for all cases.
		*/

	TCHAR strExec[MAX_PATH];
	// /C option ensures command interpreter is short-lived
	// /K forces a persistent one; here we need to switch off prompt and echoing, too
	if(CPF_REUSECMDPROC & m_dwFlags) {
		// i execute more commands to begin with using the & separator
		// but as you'd expect, this won't work in windows 9x
		if(m_bIsNT)
			// echo off /Q version is not recognized, in XP at least
			// this @echo off i specify doesn't have any effect either @@@
            _sntprintf(strExec, MAX_PATH, _T("%s /K @echo off & prompt $S$H & %s"), sys, pszCommand);
		else
			// one thought would be to use the pipe | but that would fail commands with input
			// NOTE: i'm using echo off to get rid of the prompt (!) but command echo sticks round
			_sntprintf(strExec, MAX_PATH, _T("%s /K echo off"), sys); // actual command laters
	}
	else
		_sntprintf(strExec, MAX_PATH, _T("%s /C \"%s\""), sys, pszCommand);
#ifdef _DEBUG
	ATLTRACE(_T("PIPE %x: Launching command: %s\n"), this, (LPCTSTR)strExec);
#endif

	/* LAUNCHING NON-CONSOLE STUFF
		* If you try to launch e.g a plain "notepad" then several wicked things happen. The most
		* important is that the console sticks around for as long as the "notepad" runs, and the
		* listener thread will hang till the end of this child, although it won't actually read
		* anything. The obvious solution is don't use it for launching non-console apps. But even
		* so it won't really matter as long as you don't mind having a thread or 2 waiting idle.
		@@@ if only there was a way to figure out whether the new process had a console...
		*/

	BOOL ok = CreateProcess(NULL, strExec, NULL, NULL, TRUE, // handles inherited
		dwCreateFlags, NULL, NULL, &si, &pi);

	if(ok) {
		// Close any unnecessary handles.
		CloseHandle(pi.hThread);
		m_hChildProcess = pi.hProcess;
		assert(IsChildRunning());
		m_dwProcessGroupId = pi.dwProcessId;

		if(m_bIsNT==FALSE && (CPF_REUSECMDPROC & m_dwFlags) ) {
			// issue the actual command
			_sntprintf(strExec, MAX_PATH, _T("%s\r\n"), pszCommand);
			SendChildInput(strExec);
			//SendChildInput(_T("@echo off\r\n")); ineffectual
		}
	}

	return ok;
}

// listen for output and pass it to the parent object
DWORD WINAPI CConsolePipe::ListenerThreadProc(LPVOID lpParameter)
{
	CConsolePipe* pThis = (CConsolePipe*)lpParameter;
	assert(pThis && pThis->m_hChildProcess);
	// most of the time the child process will have already terminated, but that's no rule

    // Read the child's (ansi) output.
    char lpBuffer[CPBUF_SIZE+1];
    DWORD nBytesRead;
	BOOL doOEM = !(pThis->m_dwFlags & CPF_NOCONVERTOEM);

	// keep on reading the child's stdout till the pipe is broken (child finishes)
    while(1)
    {
        if (!ReadFile(pThis->m_hOutputRead, lpBuffer, CPBUF_SIZE, 
			&nBytesRead, NULL) || nBytesRead==0)
        {
			// the usual error is ERROR_BROKEN_PIPE but don't get fussy
#ifdef _DEBUG
			DWORD err_ = GetLastError();
          if ( err_ != ERROR_BROKEN_PIPE)
              ATLTRACE(_T("MINI-assert: Listener thread broken due to error %x"), err_);
#endif
			break;
        }

		assert(nBytesRead <= CPBUF_SIZE);
		lpBuffer[nBytesRead] = 0;

#ifdef UNICODE
		// USES_CONVERSION is very bad within loops, so go manually
		WCHAR wbuf[CPBUF_SIZE+1];
		// can't rely on ES_OEMCONVERT to do the conversion as necessary
		MultiByteToWideChar( doOEM ? CP_OEMCP : CP_ACP, 
			0, lpBuffer, nBytesRead, wbuf, CPBUF_SIZE);
		wbuf[nBytesRead] = 0;
		pThis->OnReceivedOutput(wbuf);
#else
		// this is a bit dodgy since a window is updated from a non-owner thread
		if(doOEM)
			OemToChar(lpBuffer, lpBuffer); // inplace ok
		pThis->OnReceivedOutput(lpBuffer);
#endif
    }

#ifdef _DEBUG
	if( CPF_NOAUTODELETE & pThis->m_dwFlags)
		ATLTRACE(_T("PIPE %x: Listener thread terminates\n"), pThis);
	else
		ATLTRACE(_T("PIPE %x: Listener thread terminates & self-destructs\n"), pThis);
#endif

	// cleanup the thread early lest destructor forces our termination
	CloseHandle(pThis->m_hListenerThread);
	pThis->m_hListenerThread = NULL;

	// notify our parent that the child has (probably) finished
	pThis->OnReceivedOutput(NULL/*hint*/);

	if( CPF_NOAUTODELETE & pThis->m_dwFlags )
		pThis->Cleanup();
	else
		delete pThis; // hara-kiri

	return 1;
}

void CConsolePipe::Cleanup() {
	if(m_hListenerThread) {
		DWORD code;
		GetExitCodeThread(m_hListenerThread, &code);
		if(STILL_ACTIVE == code) {
			// the background thread is usually stuck in a ReadFile call
			// the only way to terminate it is to get anal with it <g>
#ifdef _DEBUG
			ATLTRACE(_T("PIPE %x: Killing listener thread\n"), this);
#endif
			TerminateThread(m_hListenerThread, 0);
		}
    // yudhi: always close handle whatever happened
		CloseHandle(m_hListenerThread);

		m_hListenerThread = NULL;
	}

	if(m_hChildProcess) {
		CloseHandle(m_hChildProcess); // leave it running though (if not already ended)
		m_hChildProcess = NULL;
	}

	if(m_hInputWrite != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hInputWrite);
		m_hInputWrite = INVALID_HANDLE_VALUE;
	}

	if(m_hOutputRead != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hOutputRead);
		m_hOutputRead = INVALID_HANDLE_VALUE;
	}

	// listener thread isn't touched; either normal termination or forced @ destruction
}

// create a guid-like unique name for what have you
// this is suitable as a generic helper
TString CConsolePipe::UGHGetUniqueName() 
{
	GUID gui;
	// here i use GUIDs; other techniques include getting GetTickCount with Thread ID etc
	HRESULT hr = CoCreateGuid(&gui); // $TSEK this don't requires CoInitialize

	LPOLESTR ole = NULL;
	hr = StringFromIID(gui, &ole); // this includes enclosing {braces}

	TString str = W2TConvert(ole); // converts as necessary
	if(ole)
		CoTaskMemFree(ole);

	return str;
}

BOOL CConsolePipe::m_bForcedConsole = FALSE;
