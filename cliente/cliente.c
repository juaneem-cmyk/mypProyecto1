#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PUERTO 1234
#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE] = {0};
    int cliente_socket;
    struct sockaddr_in direccion;
    if ((cliente_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Error al crear el socket");
    exit(EXIT_FAILURE);
    }

    // Configuro la dirección del servidor
    direccion.sin_family = AF_INET;
    direccion.sin_port = htons(PUERTO);
    direccion.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Me conecto al servidor
    if (connect(cliente_socket,
                (struct sockaddr *)&direccion,
                sizeof(direccion)) < 0) {
        perror("Error al conectar con el servidor");
        exit(EXIT_FAILURE);
    }

    printf("Conectado al servidor\n");

    char mensaje[] = "Hola servidor";

    if (send(cliente_socket, mensaje, strlen(mensaje), 0) < 0) {
        perror("Error al enviar mensaje");
        close(cliente_socket);
        exit(EXIT_FAILURE);
    }

    printf("Mensaje enviado\n");
    int bytes_recibidos = recv(cliente_socket, buffer, BUFFER_SIZE - 1, 0);

if (bytes_recibidos < 0) {
    perror("Error al recibir respuesta");
    close(cliente_socket);
    exit(EXIT_FAILURE);
}

buffer[bytes_recibidos] = '\0';

printf("Respuesta del servidor: %s\n", buffer);
    return 0;
}
/*
int bytes_recibidos = recv(nuevo_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_recibidos < 0) {
            perror("Error al recibir datos");
            close(nuevo_socket);
            continue;
        }
        buffer[bytes_recibidos] = '\0'; // Asegurarse de que el buffer esté terminado en nulo
        printf("Mensaje recibido: %s\n", buffer);

        const char respuesta[] = "Hola cliente";
        // Envío la respuesta al cliente, esto es para probar jeje
        if (send(nuevo_socket, respuesta, strlen(respuesta), 0) < 0) {
            perror("Error al enviar respuesta");
            close(nuevo_socket);
            continue;
        }
*/