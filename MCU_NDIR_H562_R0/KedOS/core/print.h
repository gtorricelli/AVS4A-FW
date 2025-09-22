/****************************************************************************
 ****************************************************************************
 *
 *                              print.h
 *
 * Author(s): R.Facchini
 * 
 * 
 * Description:
 * 
 *
 * Usage notes:
 * 
 *
 ****************************************************************************
 ****************************************************************************/

#ifndef PRINT_H
#define PRINT_H

/*-----------------------------------*
 * INCLUDE FILES
 *-----------------------------------*/
#include "types.h"



/*-----------------------------------*
 * PUBLIC DEFINES
 *-----------------------------------*/

/*  VT100 terminal macros */

/* Clear screen */
#define     CLS             "\x1b[2J"

/* cursor movement */
#define     CURPOS(X,Y)     "\x1b["#Y";"#X"H"
#define     CURFWD(C)       "\x1b["#C"C"
#define     CURBWD(C)       "\x1b["#C"D"
#define     CURUP(Y)        "\x1b["#Y"A"
#define     CURDWN(Y)       "\x1b["#Y"B"

/* text attributes */
#define     ATTR_RESET      "\x1b[0m"
#define     ATTR_BRIGHT     "\x1b[1m"
#define     ATTR_DIM        "\x1b[2m"
#define     ATTR_UNDSCR     "\x1b[4m"
#define     ATTR_BLINK      "\x1b[5m"
#define     ATTR_REV        "\x1b[7m"
#define     ATTR_HIDD       "\x1b[8m"

/* foreground color */
#define     ATTR_FBLACK     "\x1b[30m"
#define     ATTR_FRED       "\x1b[31m"
#define     ATTR_FGREEN     "\x1b[32m"
#define     ATTR_FYELLOW    "\x1b[33m"
#define     ATTR_FBLUE      "\x1b[34m"
#define     ATTR_FMAGENTA   "\x1b[35m"
#define     ATTR_FCYAN      "\x1b[36m"
#define     ATTR_FWHITE     "\x1b[37m"

/* background color */
#define     ATTR_BBLACK     "\x1b[40m"
#define     ATTR_BRED       "\x1b[41m"
#define     ATTR_BGREEN     "\x1b[42m"
#define     ATTR_BYELLOW    "\x1b[43m"
#define     ATTR_BBLUE      "\x1b[44m"
#define     ATTR_BMAGENTA   "\x1b[45m"
#define     ATTR_BCYAN      "\x1b[46m"
#define     ATTR_BWHITE     "\x1b[47m"

/* erase line */
#define     ERASE_LN        "\x1b[1K"

// ** Definizioni di formato per i numeri - vedi PrintDec() ecc. **
#define     LEAD_ZEROS              0x1
#define     NO_LEAD_ZEROS           0x0

#define     ALWAYS_SHOW_SIGN        0x2
#define     ONLY_SHOW_NEG_SIGN      0x0

// ** Definizioni di formato per le stringhe - vedi PrintFormatStr() ecc. **
#define     LEFT_ALIGNED        0
#define     RIGHT_ALIGNED       1
#define     CENTER_ALIGNED      2

// ** Aggiunta di newline a fine stringa - vedi PrintFormatStr() ecc. **
#define     NO_CRLF             0
#define     DO_CRLF             1

/*-----------------------------------*
 * PUBLIC MACROS
 *-----------------------------------*/
// None

#endif /* PRINT_H */

/****************************************************************************
 ****************************************************************************/
