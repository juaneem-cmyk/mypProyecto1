#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

/*Voy a seguir un tutorial.
 https://medium.com/@trish07/building-a-simple-tcp-chat-application-in-c-a-step-by-step-tutorial-ed3845607d16 
 Como el tutorial limita los clientes, busque otro tutorial que me sirva de guia
 https://medium.com/@shivambhadani_/understanding-tcp-and-building-our-own-tcp-server-in-c-language-8de9d9de78ef
 */

#define PUERTO 1234
#define BUFFER_SIZE 1024

int main() {
    int servidor_socket, nuevo_socket;
    struct sockaddr_in direccion;
    int opcion=1;
    char buffer[BUFFER_SIZE]= {0};
    // Creo el socket del servidor
    if ((servidor_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }
    // Configuro el socket para que pueda reutilizar la dirección y el puerto
    if (setsockopt(servidor_socket, SOL_SOCKET, SO_REUSEADDR, & opcion, sizeof(opcion)) == -1) {
        perror("Error al configurar el socket");
        exit(EXIT_FAILURE);
    }
    // Configuro la dirección del servidor
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(PUERTO);
    // Asigno la dirección al socket
    if (bind(servidor_socket, (struct sockaddr *) & direccion, sizeof(direccion)) < 0) {
        perror("Error al asignar la dirección al socket");
        exit(EXIT_FAILURE);
    }
    // Escucho conexiones entrantes
    if (listen(servidor_socket, 3) < 0) {
        perror("Error al escuchar conexiones entrantes");
        exit(EXIT_FAILURE);
    }
    printf("Servidor escuchando en el puerto %d\n", PUERTO);
    
    /* Configuro el polling para manejar múltiples clientes 
    https://www.ibm.com/docs/en/i/7.4.0?topic=designs-using-poll-instead-select */
    int capacidad = 1;
    int cantidad = 1;
    struct pollfd *fds = malloc(sizeof(struct pollfd) * capacidad);
    if (fds == NULL) {
        perror("Error al reservar memoria");
        close(servidor_socket);
        exit(EXIT_FAILURE);
    }
    fds[0].fd = servidor_socket;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    // Acepto conexiones entrantes en un bucle infinito
    while (1) {
        int actividad = poll(fds, cantidad, -1);
        if (actividad < 0) {
            perror("Error en poll");
            continue;
        }
        if (fds[0].revents & POLLIN) {
            nuevo_socket = accept(servidor_socket, NULL, NULL);
            if (nuevo_socket < 0) {
                perror("Error al aceptar la conexión");
                continue;
            }
            printf("Cliente conectado\n");

            if (cantidad == capacidad) {
                capacidad *= 2;
                struct pollfd *temporal = realloc(fds, sizeof(struct pollfd) * capacidad);
                if (temporal == NULL) {
                    perror("Error al ampliar la memoria");
                    close(nuevo_socket);
                    continue;
                }
                fds = temporal;
            }
            fds[cantidad].fd = nuevo_socket;
            fds[cantidad].events = POLLIN;
            fds[cantidad].revents = 0;
            cantidad++;
        }
        // Manejo la comunicación con los clientes conectados
        for (int i = 1; i < cantidad; i++) {
            if (fds[i].revents & POLLIN) {
                int bytes_leidos = read(fds[i].fd, buffer, BUFFER_SIZE - 1);
                if (bytes_leidos < 0) {
                    perror("Error al leer del cliente");
                    continue;
                } 
                if (bytes_leidos == 0) {
                    printf("Cliente desconectado\n");
                    close(fds[i].fd);
                    // Remuevo el cliente desconectado del arreglo de fds
                    for (int j = i; j < cantidad - 1; j++) {
                        fds[j] = fds[j + 1];
                    }
                    cantidad--;
                    i--;
                } else {
                    buffer[bytes_leidos] = '\0';
                    printf("Mensaje recibido: %s\n", buffer);
                    for (int j = 1; j < cantidad; j++) {
                        if (j != i) { // No enviar el mensaje al cliente que lo envió
                            if (send(fds[j].fd, buffer, bytes_leidos, 0) < 0) {
                                perror("Error al enviar respuesta al cliente");
                            }
                        }
                    }
                if (send(fds[i].fd, buffer, bytes_leidos, 0) < 0) {
                    perror("Error al enviar respuesta al cliente");
                }  
                }
            }
        }
    }
    return 0;
}