#include <stdio.h>
#include <stdlib.h>

int main()
{
    system("su-");
    system("chmod 777 wake_up");
    system("ls -l");
    
    printf("main finished\n");
    
    return 0;    
}