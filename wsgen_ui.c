/* vim: expandtab sw=4 sts=4 ts=8 cin
 **********************************************************
 * wsgen_ui.c
 *
 * Copyright 2004, Philippe Stellwag <linux@mp3s.name>, Germany
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Publice License,
 * version 2 or any later. The license is contained in the COPYING
 * file that comes with the wspacegen distribution.
 *
 * wspacegen ui
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "storage.h"
#include "fileio.h"
#include "getAbsPath.h"
#include "interprt.h"
#include "debug.h"


#define VERSION "0.01"

#ifdef __USE_POSIX
#  include <termio.h>
#endif

int main(void);
int openws(void);
int print(void);
int save(void);
void number(void);
void label(void);
int banner(char *menue);
int menue(void);


int main(void)
{
#ifdef __USE_POSIX
    /* turn off canonical mode, if it isn't turn off yet! 
     * 
     * FIXME
     * this is currently somewhat preliminary, 'cause it causes some 
     * incompatibilities for users of POSIX incompatible operating systems.
     * in the long run we need to allow the user to enter '1\r'!
     */
    struct termio backup, termmode;
    int termio_restore_backup = 0;

    if(! ioctl(0, TCGETA, &termmode)) {
        memmove(&backup, &termmode, sizeof(struct termio));

        termmode.c_lflag &= ~ICANON;
        termmode.c_cc[VMIN] = 1;
        termmode.c_cc[VTIME] = 0;

        if(! ioctl(0, TCSETA, &termmode))
            termio_restore_backup = 1;
    }
#endif

    menue();

#ifdef __USE_POSIX
    /* restore previous mode of canonical flag, if necessary */
    if(termio_restore_backup)
        ioctl(0, TCSETA, &backup);
#endif

    return 0;
}



/*
 * load existent ws file to stack t. m. open file (-:
 * RETURN: 0 on success, else -1 on error
 */
int openws(void)
{
    int ret = -1;
    char filename[4096 * 2];
    printf("Enter filename of ws file (CWD: %s): ",getCWD());
    scanf("%s", filename);
    printf("\n");

    if(load_file(filename))
        printf("couldn't open '%s'.\n", filename);
    else{
        printf("%s successfully loaded.\n", filename);
        ret = 0;
    }

    getchar(); getchar();

    return ret;
}



/*
 * function to print out current source code
 * TODO: print out page by page
 * RETURN: 0 on success, else error code
 */
int print(void)
{
    unsigned int i = 0;
    for (; i < wsdata_len; ++i) {
	switch (wsdata[i]) {
            case ' ':
                printf("[SPACE]");
                break;
            case '\t':
                printf("[TAB]");
                break;
            case '\n':
                printf("[LF]");
                break;
            case 0:
                putchar('\n');
                break;
	}
    }
    printf("\n\n(hit [CR])");
    getchar();
    getchar();
    return wsdata_len-i-1;
}



/*
 * save global set 'char *ws' to user defined file
 * RETURN: 0 on success, else -1
 */
int save(void)
{
    int ret = -1;
    char filename[4096 * 2];
    printf("Enter filename to save source (CWD: %s): ",getCWD());
    scanf("%s", filename);
    printf("\n");

    if(write_file(filename))
        printf("couldn't save data to '%s'.\n", filename);
    else{
        printf("%s successfully saved.\n", filename);
        ret = 0;
    }

    getchar(); getchar();

    return ret;
}



/*
 * this function converts decimal numbers to binary
 * ones and then to ws. the first character makes this
 * number positive ([SPACE]) or negativ ([TAB]). further more,
 * a 1 is replaced by [TAB] and 0 is replaced by [SPACE].
 * the return value is a combination of [SPACE]'es and [TAB]'s.
 */
void number(void)
{
    char buf[64];
    int dec;
    int bitmask = 1;

    printf("type number (int): ");
    /* scanf("%d", &dec);
     * 
     * this unfortunately leaves an \n in the input buffer, which then would
     * have to be treated by the main menu, however that's not what we fuckin'
     * want to have!
     */
    if(! fgets(buf, sizeof(buf), stdin))
        dec = 0; /* force returning a zero, 'cause we currently have to return
                  * something, unless we'd confuse our wsdata stack.
                  * probably there's something terrible going on anyways ...
                  */
    else
        dec = atoi(buf);

    /* write out sign bit, \t => negativ, ' ' => positive */
    compose_require(1);
    compose_push((dec < 0) ? '\t' : ' ');

    /* fixed: calculate with absolute value from here on! */
    dec = dec < 0 ? (-1 * dec) : dec;

    for(;bitmask <= dec; bitmask <<= 1);
    bitmask >>= 1;
    
    /* convert to binary */
    do {
    	/* write out next bit, 0 => ' ' or 1 => '\t' */
        compose_require(1);
    	compose_push((dec & bitmask) ? '\t' : ' ');
    } while ((bitmask >>= 1));

    compose_require(1);
    compose_push('\n'); /* number ends with a [LF] */
}



/*
 * labels are a free combination of [TAB]'s and [SPACE]'es followed
 * by a [LF]. the return value of label() is a combination of [TAB]'s
 * and [SPACE]'es plus a [LF] on the end.
 */
void label(void)
{
    char lbl[512];
    int lenlbl;
    int i = 0;

    printf("Enter label ('t' for [TAB] and 's' for [SPACE] in\n");
    printf("  an arbitrary combination): ");
    /* scanf("%s", lbl);
     * 
     * scanf unfortunately leave the \n at the end of the row in the
     * input buffer, thus hurting our mainmenu. need to look here
     * when more time's on my side
     * for now: FIXME
     */
    if(! fgets(lbl, sizeof(lbl), stdin))
        *lbl = 0; /* we need to return something (at least a \n), in 
                   * order to not confuse our wsdata stack.
                   */

    lenlbl = strlen(lbl);

    for (; i < lenlbl; ++i) {
        compose_require(1);
        
	switch (lbl[i]) {
	case 't':
            compose_push('\t');
	    break;
	case 's':
            compose_push(' ');
	    break;
	}
    }

    compose_require(1);
    compose_push('\n');
}



/*
 * the function banner(...) print out a header followed by a section name
 * (char *menue). be careful, every call cleans your screen (-:
 */
int banner(char *menue)
{
    int lenmenue = strlen(menue) + 8;
    int i = 0;

    #ifdef _WIN32
        system("cls");
    #else
        /* output ANSI clear sequence instead of spawning a shell,
         * which executes UNIX clear command
         *   system("clear");
         */
        {
            char clear[] = { 0x1b, 0x5b, 0x48, 0x1b, 0x5b, 0x4a, 0 };
            printf("%s", clear);
        }
    #endif

    printf("  +------------------------------------------------------+\n");
    printf("  |                   wspacegen v%s                    |\n", VERSION);
    printf("  |    (c)2004 by Philippe Stellwag <linux@mp3s.name>,   |\n");
    printf("  |               Stefan Siegl <ssiegl@gmx.de>           |\n");
    printf("  |                                                      |\n");
    printf("  | get whitespace: http://compsoc.dur.ac.uk/whitespace/ |\n");
    printf("  +------------------------------------------------------+\n\n");

    printf("section: %s\n", menue);

    for (; i <= lenmenue; ++i)
	printf("-");
    printf("\n\n");

    return 0;
}




/*
 * this is the main menue, controlls everything (user controlled):
 * call functions, ask the users, print out misc menues, ...
 */
int menue(void)
{
    char mi[1];			/* user input main menue */
    char si[1];			/* user input 'stack manipulation' menue */
    char ai[1];			/* user input 'arithmetic' menue */
    char hi[1];			/* user input 'heap access' menue */
    char fi[1];			/* user input 'flow control' menue */
    char ii[1];			/* user input 'I/O' menue */

    do {
	banner("main menue");

	printf("CMD  IMP            MEANING\n");
	printf("---  ---            -------\n");
	printf("(1)  [SPACE]        stack manipulation\n");
	printf("(2)  [TAB][SPACE]   arithmetic\n");
	printf("(3)  [TAB][TAB]     heap access\n");
	printf("(4)  [LF]           flow control\n");
	printf("(5)  [TAB][LF]      I/O\n\n");
	printf("(p)                 print out source code\n");
	printf("(o)                 open file\n");
	printf("(e)                 edit source code\n");
	printf("(d)                 run source code in debug mode\n");
	printf("(r)                 run source code\n");
	printf("(s)                 save source code to file\n");
	printf("(x)                 end wspacegen\n\n");

	printf("What would you do (CMD)? ");
	scanf("%c", &mi[0]);
	printf("\n");

	switch (mi[0]) {
            case '1':
                do {
                    banner("stack manipulation (IMP: [SPACE])");

                    printf("CMD  Command        Parameters        Meaning\n");
                    printf("---  -------        ----------        -------\n");
                    printf("(1)  [SPACE]        number            push the number onto the stack\n");
                    printf("(2)  [LF][SPACE]                      duplicate the top item on the stack\n");
                    printf("(3)  [TAB][SPACE]   number            copy the *n*th item on the stack\n");
                    printf("                                      (given by the argument) onto the\n");
                    printf("                                      top of the stack\n");
                    printf("(4)  [LF][TAB]                        swap the top two items on the stack\n");
                    printf("(5)  [LF][LF]                         discard the top item on the stack\n");
                    printf("(6)  [TAB][LF]      number            slide n items off the stack, keeping\n");
                    printf("                                      the top item\n\n");
                    printf("(m)                                   return to main menue\n\n");

                    printf("What would you do (CMD)? ");
                    scanf("%c", &si[0]);
                    printf("\n");

                    switch (si[0]) {
                        case '1': /* [SPACE][SPACE] */
                            compose_require(4);
                            compose_push(' ');
                            compose_push(' ');
                            number();
                            compose_append_to_wsdata();
                            break;
                            
                        case '2': /* [SPACE][LF][SPACE] */
                            compose_require(3);
                            compose_push(' ');
                            compose_push('\n');
                            compose_push(' ');
                            compose_append_to_wsdata();
                            break;
                            
                        case '3': /* [SPACE][TAB][SPACE] */
                            compose_require(5);
                            compose_push(' ');
                            compose_push('\t');
                            compose_push(' ');
                            number();
                            compose_append_to_wsdata();
                            break;
                            
                        case '4': /* [SPACE][LF][TAB] */
                            compose_require(3);
                            compose_push(' ');
                            compose_push('\n');
                            compose_push('\t');
                            compose_append_to_wsdata();
                            break;
                            
                        case '5': /* [SPACE][LF][LF] */
                            compose_require(3);
                            compose_push(' ');
                            compose_push('\n');
                            compose_push('\n');
                            compose_append_to_wsdata();
                            break;
                            
                        case '6': /* [SPACE][TAB][LF] */
                            compose_require(5);
                            compose_push(' ');
                            compose_push('\t');
                            compose_push('\n');
                            number();
                            compose_append_to_wsdata();
                            break;

                        case 'm':
                            /* do nothing */
                            break;
                    }
                } while (si[0] != 'm');
                break;
            case '2':
                do {
                    banner("arithmetic (IMP: [TAB][SPACE])");

                    printf("CMD  Command        Parameters        Meaning\n");
                    printf("---  -------        ----------        -------\n");
                    printf("(1)  [SPACE][SPACE]                   addition\n");
                    printf("(2)  [SPACE][TAB]                     subtraction\n");
                    printf("(3)  [SPACE][LF]                      multiplication\n");
                    printf("(4)  [TAB][SPACE]                     integer division\n");
                    printf("(5)  [TAB][TAB]                       modulo\n\n");
                    printf("(m)                                   return to main menue\n\n");

                    printf("What would you do (CMD)? ");
                    scanf("%c", &ai[0]);
                    printf("\n");

                    switch (ai[0]) {
                        case '1': /* [TAB][SPACE][SPACE][SPACE] */
                            compose_require(4);
                            compose_push('\t');
                            compose_push(' ');
                            compose_push(' ');
                            compose_push(' ');
                            compose_append_to_wsdata();
                            break;

                        case '2': /* [TAB][SPACE][SPACE][TAB] */
                            compose_require(4);
                            compose_push('\t');
                            compose_push(' ');
                            compose_push(' ');
                            compose_push('\t');
                            compose_append_to_wsdata();
                            break;

                        case '3': /* [TAB][SPACE][SPACE][LF] */
                            compose_require(4);
                            compose_push('\t');
                            compose_push(' ');
                            compose_push(' ');
                            compose_push('\n');
                            compose_append_to_wsdata();
                            break;

                        case '4': /* [TAB][SPACE][TAB][SPACE] */
                            compose_require(4);
                            compose_push('\t');
                            compose_push(' ');
                            compose_push('\t');
                            compose_push(' ');
                            compose_append_to_wsdata();
                            break;

                        case '5': /* [TAB][SPACE][TAB][TAB] */
                            compose_require(4);
                            compose_push('\t');
                            compose_push(' ');
                            compose_push('\t');
                            compose_push('\t');
                            compose_append_to_wsdata();
                            break;

                        case 'm':
                            /* do nothing */
                            break;
                    }

                } while (ai[0] != 'm');
                break;

            case '3':
                do {
                    banner("heap access (IMP: [TAB][TAB])");

                    printf("CMD  Command        Parameters        Meaning\n");
                    printf("---  -------        ----------        -------\n");
                    printf("(1)  [SPACE]                          store\n");
                    printf("(2)  [TAB]                            retrieve\n\n");
                    printf("(m)                                   return to main menue\n\n");

                    printf("What would you do (CMD)? ");
                    scanf("%c", &hi[0]);
                    printf("\n");

                    switch (hi[0]) {
                        case '1': /* [TAB][TAB][SPACE] */
                            compose_require(3);
                            compose_push('\t');
                            compose_push('\t');
                            compose_push(' ');
                            compose_append_to_wsdata();
                            break;

                        case '2': /* [TAB][TAB][TAB] */
                            compose_require(3);
                            compose_push('\t');
                            compose_push('\t');
                            compose_push('\t');
                            compose_append_to_wsdata();
                            break;

                        case 'm':
                            /* do nothing */
                            break;
                    }
                } while (hi[0] != 'm');
                break;
            case '4':
                do {
                    banner("flow control (IMP: [LF])");

                    printf("CMD  Command        Parameters        Meaning\n");
                    printf("---  -------        ----------        -------\n");
                    printf("(1)  [SPACE][SPACE] label             mark a location in the program\n");
                    printf("(2)  [SPACE][TAB]   label             call a subroutine\n");
                    printf("(3)  [SPACE][LF]    label             jump unconditionally to a label\n");
                    printf("(4)  [TAB][SPACE]   label             jump to a label if the top of the stack is zero\n");
                    printf("(5)  [TAB][TAB]     label             jump to a label if the top of the stack is negativ\n");
                    printf("(6)  [TAB][LF]                        end a subroutine and transfer control back to the caller\n");
                    printf("(7)  [LF][LF]                         end the progam\n\n");
                    printf("(m)                                   return to main menue\n\n");

                    printf("What would you do (CMD)? ");
                    scanf("%c", &fi[0]);
                    printf("\n");

                    switch (fi[0]) {
                        case '1': /* [LF][SPACE][SPACE] */
                            compose_require(4);
                            compose_push('\n');
                            compose_push(' ');
                            compose_push(' ');
                            label();
                            compose_append_to_wsdata();
                            break;

                        case '2': /* [LF][SPACE][TAB] */
                            compose_require(4);
                            compose_push('\n');
                            compose_push(' ');
                            compose_push('\t');
                            label();
                            compose_append_to_wsdata();
                            break;

                        case '3': /* [LF][SPACE][LF] */
                            compose_require(4);
                            compose_push('\n');
                            compose_push(' ');
                            compose_push('\n');
                            label();
                            compose_append_to_wsdata();
                            break;

                        case '4': /* [LF][TAB][SPACE] */
                            compose_require(4);
                            compose_push('\n');
                            compose_push('\t');
                            compose_push(' ');
                            label();
                            compose_append_to_wsdata();
                            break;

                        case '5': /* [LF][TAB][TAB] */
                            compose_require(3);
                            compose_push('\n');
                            compose_push('\t');
                            compose_push('\t');
                            label();
                            compose_append_to_wsdata();
                            break;

                        case '6': /* [LF][TAB][LF] */
                            compose_require(3);
                            compose_push('\n');
                            compose_push('\t');
                            compose_push('\n');
                            compose_append_to_wsdata();
                            break;

                        case '7': /* [LF][LF][LF] */
                            compose_require(3);
                            compose_push('\n');
                            compose_push('\n');
                            compose_push('\n');
                            compose_append_to_wsdata();
                            break;

                        case 'm':
                            /* do nothing */
                            break;
                    }
                } while (fi[0] != 'm');
                break;
            case '5':
                do {
                    banner("I/O (IMP: [TAB][LF])");

                    printf("CMD  Command        Parameters        Meaning\n");
                    printf("---  -------        ----------        -------\n");
                    printf("(1)  [SPACE][SPACE]                   output the character at the top of the stack\n");
                    printf("(2)  [SPACE][TAB]                     output the number at the top of the stack\n");
                    printf("(3)  [TAB][SPACE]                     Read a character and place it in the location\n");
                    printf("                                      given by the top of the stack\n");
                    printf("(4)  [TAB][TAB]                       Read a number and place it in the location\n");
                    printf("                                      given by the top of  the stack\n\n");
                    printf("(m)                                   return to main menue\n\n");

                    printf("What would you do (CMD)? ");
                    scanf("%c", &ii[0]);
                    printf("\n");

                    switch (ii[0]) {
                        case '1': /* [TAB][LF][SPACE][SPACE] */
                            compose_require(4);
                            compose_push('\t');
                            compose_push('\n');
                            compose_push(' ');
                            compose_push(' ');
                            compose_append_to_wsdata();
                            break;

                        case '2': /* [TAB][LF][SPACE][TAB] */
                            compose_require(4);
                            compose_push('\t');
                            compose_push('\n');
                            compose_push(' ');
                            compose_push('\t');
                            compose_append_to_wsdata();
                            break;

                        case '3': /* [TAB][LF][TAB][SPACE] */
                            compose_require(4);
                            compose_push('\t');
                            compose_push('\n');
                            compose_push('\t');
                            compose_push(' ');
                            compose_append_to_wsdata();
                            break;

                        case '4': /* [TAB][LF][TAB][TAB] */
                            compose_require(4);
                            compose_push('\t');
                            compose_push('\n');
                            compose_push('\t');
                            compose_push('\t');
                            compose_append_to_wsdata();
                            break;

                        case 'm':
                            /* do nothing */
                            break;
                    }
                } while (ii[0] != 'm');
                break;
            case 'p':
                #ifdef _WIN32
                    system("cls");
                #else
                    system("clear");
                #endif
                banner("current code");
                print();
                break;
            case 'o':
                /* open file */
                openws();
                break;
            case 'e':
                /* edit source code */
                printf("'edit mode' currently not implemented )-:\n");
                getchar();
                getchar();
                break;
                
            case 'd':
                /* run debugger */
                debug_launch(); 
                break;

            case 'r':
                /* run source code */
                interprt_init(); 
                interprt_err_handler(stdout, interprt_cont());
                printf("\n\nhit any key to continue.\n");
                getchar();
                getchar();
                break;
            case 's':
                /* save source code to file */
                save();
                break;
            case 'x':
                /* do nothing */
                break;
	}
    } while (mi[0] != 'x');

    return 1;
}



/***** -*- emacs is great -*-
Local Variables:
mode: C
c-basic-offset: 4
indent-tabs-mode: nil
end: 
****************************/
