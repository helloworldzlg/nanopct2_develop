#include <stdio.h>
#include <stdlib.h>

int main()
{
    system("sudo chmod 777 wake_up");
    system("ls -l");
    
    printf("main finished\n");
    
    return 0;    
}