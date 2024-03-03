#include <stdio.h>

#include "handlers/handlers_export.h"
#include "server.h"


int main()
{
    SERVER chat_room = server_create();

    server_set_path(&chat_room, "/", root_handler);
    server_set_path(&chat_room, "/*", rooms_handler);
    server_set_path(&chat_room, "/404", not_found_handler);

    // server_start(&chat_room, 8080);
    server_start_thread(&chat_room, 8080);
}
