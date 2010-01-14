#ifndef UPRIGHT_CONSOLEPIPE_WTL
#define UPRIGHT_CONSOLEPIPE_WTL

#include "../../core/TString.h"

#define ATLTRACE
/////////////////////////////////////////////////////////////////////////////
// consolePipe.h by Nikos Bozinis @ 19/09/2002 07:19:01
// Self-contained class which launches a console-based application and 
// receives its output, which it then emits into a specified edit window.
//
// NOTE: THIS SHOULD ALWAYS BE CREATED ON THE HEAP, IT SELF-DESTRUCTS! (see below)
// This class ain't designed to be reusable; if need be just create a fresh
// one for every DOS command you want executed.
// P.S. The new CPF_NOAUTODELETE flag allows this class to be reused after all
//
// LICENCE AGREEMENT
// You are free to use this class or modifications thereof in your code.
// If you do so, a mention of the original author in your product credits
// section would be expected/appreciated.
//
// http://netez.com/2xExplorer


// unresolved matters:
// * what if main program exits while this is still executing?

// bits for customizing behaviour
#define CPF_CAPTURESTDOUT (1<<0)
#define CPF_TRACEINSTANCE (1<<1) /* useful when many commands printf in the same console */
#define CPF_REUSECMDPROC  (1<<2) /* use a persistent command processor -- /K instead of /C */
#define CPF_NOAUTODELETE  (1<<3) /* no self-destruction when child terminates */
#define CPF_NOCONVERTOEM  (1<<4) /* do not assume that the processor is OEM-only */
// could add options for non-cmd processors, but it is dodgy unicode-wise


// return codes from Execute method
// A CPEXEC_MISC_ERROR return indicates that perhaps command may be executed via other means
// (e.g. a plain CreateProcess)
#define CPEXEC_OK            0 /* no error */
#define CPEXEC_COMMAND_ERROR 1 /* command is badly formed or can't be found */
#define CPEXEC_MISC_ERROR    2 /* either console, thread or piping error */

// buffer used by listener thread (change to taste)
#define CPBUF_SIZE 255


/////////////////////////////////////////////////////////////////////////////
// CConsolePipe
// derive & override OnReceivedOutput for customization

// let me reiterate: allocate instances on the heap (new), not stack
class CConsolePipe
{
public:
	CConsolePipe(DWORD flags);
	virtual ~CConsolePipe();

	/// stop the background process and cleanup
	/// DON'T USE THIS METHOD UNLESS YOU HAVE EXHAUSTED ALL OTHER POSSIBLE EXIT ROUTES
	/// this is of limited value since main app can't tell if this class has already self-destructed!
	/// if a cached pointer of this class is used, better use IsBadReadPtr to arse-cover yourself
	void Break();

	/// call this when your (GUI) application exits to cleanup any forced consoles
	static void Term();

	/// at most one console is attached per process, this tests whether this is the case
	static BOOL IsConsoleAttached();

	/// execute a command with a DOS command processor; see CPEXEC_xxx for return values
	int Execute(LPCTSTR pszCommand);

	/// override this for doing other things with the output
	virtual void OnReceivedOutput(LPCTSTR pszText) = 0;

	/// write data to be read by child process from its stdin
	/// useful for commands that expect user input e.g. Y/N answers
	BOOL SendChildInput(LPCTSTR pszText);

	BOOL SendChildInput(void *pData, DWORD dwSize);

	/// see comments in Break(); ensure that this instance ain't deleted first!
	BOOL IsChildRunning();

	/// for persistent command processors, send a command that will terminate cmd.exe
	/// typically this will be called before the main app exits
	void StopCmd();

	/// soft-break for processes like "more"
	void SendCtrlBrk();

private: // internal implementation
	BOOL SetupConsole();
	BOOL LaunchRedirected(LPCTSTR pszCommand, HANDLE hChildStdOut, HANDLE hChildStdIn, 
		HANDLE hChildStdErr);
	// listen for output and pass it to the parent object
	static DWORD WINAPI ListenerThreadProc(LPVOID lpParameter);

	void Cleanup();

	// create a guid-like unique name for what have you
	// this is suitable as a generic helper
    static TString UGHGetUniqueName();

public:
	const DWORD m_dwFlags; // see CPF_xxx definitions; available but only for "read-only"

protected:
	const BOOL m_bIsNT; // several differences exist between NT/9x piping
	HANDLE m_hListenerThread; // reads stdout side of pipe for new printouts from child
	HANDLE m_hChildProcess; // the one we launch
	// handles for OUR end of pipe for child's stdin and stdout, respectively
	HANDLE m_hInputWrite, m_hOutputRead;
	DWORD m_dwProcessGroupId; // in case we need to send any Ctrl-C events

	static BOOL m_bForcedConsole; // whether we forced AllocConsole, typically for win9x


#ifdef _DEBUG
private:
	CConsolePipe(); // no casual construction
	CConsolePipe(CConsolePipe& copy); // disallow copy assignment etc
   void operator=(CConsolePipe& assgn);
#endif
};

#endif // #ifndef UPRIGHT_CONSOLEPIPE_WTL
