/**************************************************************
 *
 * MFUTILS.C : mfutils.c - standard code utilites I use while 
 *             programming
 *
 *
 **************************************************************
 *
 * REVISION HISTORY:
 *
 * DATE                 .Comment                       .AUTHOR
 * ============================================================
 *
 * 12-04-2005           Initial revison  -           M.Fatheree   
 *                  added read_word, 
 **************************************************************/
 
#include "mfutils.h" 
 
 
int read_word(int fd, char *buf, int maxlen)
{
    int i=0;
    int ok=1;
    int rd;
    char ch;
    int end = 0;
    
    while( ok ){
        rd = read(fd, &ch, 1);
        if( rd > 0 ){
            switch( ch ){
                case ' ':
                case 0x0d:
                case 0x0a:
                    if( i > 0){
                        ok = 0;
                        buf[i] = 0;
                    }
                break;
                default:
                    buf[i] = ch;
                    i++;
                }
        }
        else{
            ok = 0;
        }
    }
    return i;
}
 
 
