#ifndef MSGBOX_H
#define MSGBOX_H


int MessageBox(char * string, U32 msgstyle);
#define	RET_NONE 	  0
#define	RET_CANCEL	1
#define	RET_OK		  2
#define	RET_YES		  3
#define	RET_NO		  4
#define	RET_HELP	  5

// Predefined Message Dialog Styles
#define ERROR_STYLE        0x01
#define WARNING_STYLE      0x02
#define YESNOCANCEL_STYLE  0x03
#define OKCANCEL_STYLE     0x04
#define YESNO_STYLE        0x05

#endif

