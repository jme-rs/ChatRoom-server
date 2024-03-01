#include <stdio.h>

#include "handlers/handlers_export.h"
#include "server.h"


int main()
{
    SERVER server = server_create();

    server_set_path(&server, "/", root_handler);
    server_set_path(&server, "/*", rooms_handler);
    server_set_path(&server, "/404", not_found_handler);

    server_start(&server, 8080);
}
