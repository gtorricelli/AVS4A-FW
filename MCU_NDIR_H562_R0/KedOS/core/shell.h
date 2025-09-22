/*
 * shell.h
 *
 *  Created on: 04/dic/2011
 *      Author: Rodolfo Facchini
 */

#ifndef SHELL_H_
#define SHELL_H_

/*-----------------------------------*
 * INCLUDE FILES
 *-----------------------------------*/

#include "types.h"
#include "print.h"

char * ConsoleStrTok( char *strToken );
char * ShellProcess();
void   ShellInit();
void   ShellEnable();
void   ShellDisable();
int    getShellProcessChar();
void   launch_shell_process(void* func);
void   printshell();
int    __io_putchar(int ch);
int    __io_getchar(void);



#endif /* SHELL_H_ */
