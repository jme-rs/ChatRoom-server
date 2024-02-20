#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "server.h"


int main()
{
    server(8080);
    return 0;
}
