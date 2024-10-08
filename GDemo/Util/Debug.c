#include "debug.h"
 
#pragma warning(disable : 4201 4214 4115)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning(default : 4201 4214 4115; disable : 4514)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
static char debug_s[1000];
static int dead = 0;
 
void assume_message_and_exit(char *str, char *file, long line)
{
	long last_error;
	if (dead)
		return;
	dead = 1;
	last_error = GetLastError();
	sprintf(debug_s, "A failure condition was detected:\n\"%s\" in file %s, at line %ld\n(GetLastError=%ld)",
		str, file, line, last_error);
	MessageBox(NULL, debug_s, "Failure Condition", MB_OK | MB_TASKMODAL);
	exit(-2345);
}
 
 
#ifdef NDEBUG
	 // assertions are disabled
	 // (but not assumptions)
 
#else
 
 
void assert_message_and_exit(char *str, char *file, long line)
{
	long last_error;
	if (dead)
		return;
	dead = 1;
	last_error = GetLastError();
	sprintf(debug_s, "%s in file %s, at line %ld\n(GetLastError=%ld)",
		str, file, line, last_error);
	MessageBox(NULL, debug_s, "Assertion Failed", MB_OK | MB_TASKMODAL);
	exit(-1234);
}
 
 
#endif
 
