#ifndef __DEBUG_MODULE_HEADER__
#define __DEBUG_MODULE_HEADER__
 
// There is no reason to override assert with identical functionality!
#include <assert.h>
 
#ifdef __cplusplus
extern "C" {
#endif
 
 
void assume_message_and_exit(char *str, char *file, long line);
#define assume(p)	((p) ? (void)0 : assume_message_and_exit((char *)#p, (char *)__FILE__, (long)__LINE__))
 
#ifdef NDEBUG
 
//#define assert(expression)
#define DEBUG_RVAL(expression,test_expression) expression
 
 
#else
 
 
#if defined(WIN32) || defined(__WIN32__)
#define RVALTYPE unsigned int
#else
#define RVALTYPE unsigned long
#endif
 
void assert_message_and_exit(char *str, char *file, long line);
//#define assert(p)   ((p) ? (void)0 : assert_message_and_exit((char *)#p, (char *)__FILE__, (long)__LINE__))
#define RVAL (RVALTYPE)rval
#define DEBUG_RVAL(p,check_exp) { \
		   RVALTYPE rval; \
		   rval = (RVALTYPE)p; \
		   (check_exp ? (void)0 : assert_message_and_exit((char *)#p, (char *)__FILE__, (long)__LINE__)) \
		   }
 
#endif								   // not NDEBUG
 
#ifdef __cplusplus
}
#endif
#endif
 
 
