#ifndef oemaple_h
#define oemaple_h
#define IDM_ABOUT 100

int PASCAL WinMain( HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine,
		    int nCmdShow );
WORD InitApplication( HANDLE hInstance );
WORD InitInstance( HANDLE hInstance, int nCmdShow );
int ReadLineFromFile( int fd, char *buf );
void WriteLineToFile( char *p );
long FAR PASCAL MainWndProc( HWND hWnd, UINT message, WPARAM wParam,
			     LPARAM lParam );
WORD FAR PASCAL About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
#endif
