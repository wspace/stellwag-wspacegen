/*
    ###########################################################################################
    #                                                                                         #
    #           getAbsPath.h (modified) - convert a relative path to an absolute              #
    #                                                                                         #
    #           (c)2004 by Philippe Stellwag                                                  #   
    #                                                                                         #
    #           Author                                                                        #
    #                 Philippe Stellwag <linux@mp3s.name>                                     #
    #           Description                                                                   #
    #                 convert a relative path to an absolute (win32/unix)                     #
    #                 using CWD and relative path as basis                                    #
    #           License                                                                       #
    #                 This program is free software; you can redistribute it and/or modify    #
    #                 it under the terms of the GNU General Public License as published by    #
    #                 the Free Software Foundation; either version 2 of the License, or       #
    #                 (at your option) any later version.                                     #
    #                                                                                         #
    #           Downloaded from http://www.sourceforge.net/projects/getabspath/               #
    #                                                                                         #
    # --------------------------------------------------------------------------------------- #
    #                                                                                         #
    # VERSION   NAME                  DATE         DESCRIPTION                                #
    # 1.0       Philippe Stellwag     2004-05-01   creation                                   #
    # 1.1       Philippe Stellwag     2004-05-12   make some code faster                      #
    # 1.2       Philippe Stellwag     2004-05-15   include _MAX_PATH (win32)                  #
    # 1.3*      Philippe Stellwag     2004-07-13   convert program to c header file           #
    #                                                                                         #
    # --------------------------------------------------------------------------------------- #
    #                                                                                         #
    # KNOWN BUGS                                                                              #
    #   email bugs to linux@mp3s.name in English or German                                    #
    #                                                                                         #
    ###########################################################################################
*/

#ifdef _WIN32
    #include <direct.h>
    #include <stdlib.h>
#else
    #include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    int win32 = 1;
#else
    int win32 = 0;
#endif

char cwdPath[4096];

char *getCWD(void);
char *getAbsPath(char *relPath);


/*
 * // ***** sample *****
 *
 * int main(int argc,char* argv[]);
 * 
 * int main(int argc,char* argv[])
 * {
 *    if(argc!=2){
 *        fprintf(stderr,"wrong arguments, argv[1] must be an relative or absolute path\n");
 *        return 5;
 *    }
 *
 *    printf("argv[1]: %s\n",argv[1]);
 *    printf("getAbsPath: %s\n\n",getAbsPath(argv[1]));
 *
 *    return 0;
 * }
 */



/*
 * get the current working directory
 * RETURN: NULL on error, else CWD as (char *)
 */
char *getCWD(void)
{
    #ifdef _WIN32
        return _getcwd(cwdPath,4096);
    #else
        return getcwd(cwdPath,4096);
    #endif
        
    return NULL; /* I'll hope this will never happens (-: */
}




/*
 * convert an relative path to an absolute one
 * RETURN: (char *) relPath on error, else an absolute path
 */
char *getAbsPath(char *relPath)
{
    int sizeRelPath = strlen(relPath);
    int lastState = 0;
    int c, i, u, o, l;

    printf("relPath: %s\n",relPath);
    
    #ifdef _WIN32
        if(_getcwd(cwdPath,4096)==NULL)
        {
            fprintf(stderr,"getAbsPath.h: _getcwd error\n");
            return relPath;
        }
    #else
        if(getcwd(cwdPath,4096)==NULL)
        {
            fprintf(stderr,"getAbsPath.h: getcwd error\n");
            return relPath;
        }
    #endif

    if(win32==1) /* WIN32 */
    {
        for(c=0;c<sizeRelPath;c++) if(relPath[c]=='/') relPath[c]='\\';

        if(relPath[1]==':') return relPath;
        if(strncmp(relPath,"\\\\",2)==0) return relPath;
        
        for(i=0;i<sizeRelPath;i++)
        {
            if(relPath[i]=='\\')
            {
                char curPath[4096];
                int j = 0;

                for(l=0;l<4096;l++) curPath[l]='\0';

                for(j=lastState;j<=i;j++)
                {
                    curPath[j-lastState] = relPath[j];
                }
                
                curPath[j] = '\0';
                lastState = j;

                if(strcmp(curPath,"..\\")==0)
                {
                    int d = strlen(cwdPath);

                    while(cwdPath[d]!='\\') d--;

                    cwdPath[d] = '\0';
                }else if(strcmp(curPath,".\\")!=0){
                    if(cwdPath[strlen(cwdPath)-1]!='\\') strcat(cwdPath,"\\");

                    strcat(cwdPath,curPath);

                    if(cwdPath[strlen(cwdPath)-1]=='\\') cwdPath[strlen(cwdPath)-1]='\0';
                }
            }
        }

        for(u=strlen(cwdPath);u<4096;u++) cwdPath[u]='\0';

        if(cwdPath[strlen(cwdPath)-1]!='\\') strcat(cwdPath,"\\");

        for(o=lastState;o<sizeRelPath;o++) cwdPath[strlen(cwdPath)]=relPath[o];
        
        cwdPath[strlen(cwdPath)] = '\0';

    }else{ /* UNIX */

        if(relPath[0]=='/') return relPath;

        for(i=0;i<sizeRelPath;i++)
        {
            if(relPath[i]=='/')
            {
                char curPath[4096];
                int j = 0;

                for(l=0;l<4096;l++) curPath[l]='\0';

                for(j=lastState;j<=i;j++)
                {
                    curPath[j-lastState] = relPath[j];
                }
                
                curPath[j] = '\0';
                lastState = j;

                if(strcmp(curPath,"../")==0)
                {
                    int d = strlen(cwdPath);

                    while(cwdPath[d]!='/') d--;

                    cwdPath[d] = '\0';
                }else if(strcmp(curPath,"./")!=0){
                    if(cwdPath[strlen(cwdPath)-1]!='/') strcat(cwdPath,"/");

                    strcat(cwdPath,curPath);

                    if(cwdPath[strlen(cwdPath)-1]=='/') cwdPath[strlen(cwdPath)-1]='\0';
                }
            }
        }

        for(u=strlen(cwdPath);u<4096;u++) cwdPath[u]='\0';

        if(cwdPath[strlen(cwdPath)-1]!='/') strcat(cwdPath,"/");

        for(o=lastState;o<sizeRelPath;o++) cwdPath[strlen(cwdPath)]=relPath[o];
        
        cwdPath[strlen(cwdPath)] = '\0';
    }

    return cwdPath;
}



/***** -*- emacs is great -*-
Local Variables:
mode: C
c-basic-offset: 4
indent-tabs-mode: nil
end: 
****************************/
