#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*Voy a seguir un tutorial.
 https://medium.com/@trish07/building-a-simple-tcp-chat-application-in-c-a-step-by-step-tutorial-ed3845607d16 */

#define PUERTO 1234
#define BUFFER_SIZE 1024
