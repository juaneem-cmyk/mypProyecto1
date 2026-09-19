#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <signal.h>
#include "Protocolo.h"

/*Voy a seguir un tutorial.
 https://medium.com/@trish07/building-a-simple-tcp-chat-application-in-c-a-step-by-step-tutorial-ed3845607d16 
 Como el tutorial limita los clientes, busque otro tutorial que me sirva de guia
 https://medium.com/@shivambhadani_/understanding-tcp-and-building-our-own-tcp-server-in-c-language-8de9d9de78ef
 */

#define PUERTO 1234
#define BUFFER_SIZE 1024

// Cerrar adecuadamente el servidor, recomendación de Canek
volatile sig_atomic_t servidor_activo = 1;
void manejar_señal(int señal) {
    servidor_activo = 0;
}

// Enviar respuesta a los clientes conectados
void enviar_mensaje (int socket, const char *mensaje) {
    send(socket, mensaje, strlen(mensaje), 0);
}

struct cliente {
    int socket;
    int usados;
    char buffer[BUFFER_SIZE];
    char nombre_usuario[9];
    int identificado;
};

int main() {
    int servidor_socket, nuevo_socket;
    struct sockaddr_in direccion;
    int opcion=1;
    signal(SIGINT, manejar_señal); // Manejo de la señal SIGINT para cerrar el servidor adecuadamente
    
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
    struct cliente *clientes = malloc(sizeof(struct cliente) * capacidad);
    
    // Inicializo los clientes
    if (clientes == NULL) {
        perror("Error al reservar memoria para clientes");
        free(fds);
        close(servidor_socket);
        exit(EXIT_FAILURE);
    }
    if (fds == NULL) {
        perror("Error al reservar memoria");
        free(clientes);
        close(servidor_socket);
        exit(EXIT_FAILURE);
    }
    fds[0].fd = servidor_socket;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    
    // Acepto conexiones entrantes en un bucle infinito
    while (servidor_activo) {
        int actividad = poll(fds, cantidad, 1000); // Espera hasta 1 segundo para actividad
        if (actividad < 0) {
            perror("Error en poll");
            break;
        }
        if (actividad == 0) {
            // No hay actividad, continúo esperando
            continue;
        }
        
        // Si hay actividad en el socket del servidor, acepto la conexión entrante
        if (fds[0].revents & POLLIN) {
            nuevo_socket = accept(servidor_socket, NULL, NULL);
            if (nuevo_socket < 0) {
                perror("Error al aceptar la conexión");
                continue;
            }
            printf("Cliente conectado\n");
            
            // Si la cantidad de clientes alcanza la capacidad, duplico la capacidad
            if (cantidad == capacidad) {
                capacidad *= 2;
                struct pollfd *temporal = realloc(fds, sizeof(struct pollfd) * capacidad);
                if (temporal == NULL) {
                    perror("Error al ampliar la memoria");
                    close(nuevo_socket);
                    continue;
                }
                fds = temporal;
                struct cliente *temporal_clientes = realloc(clientes, sizeof(struct cliente) * capacidad);
                
                // Verifico si la memoria se amplió correctamente
                if (temporal_clientes == NULL) {
                    perror("Error al ampliar la memoria para clientes");
                    close(nuevo_socket);
                    continue;
                }
                clientes = temporal_clientes;
            }

            fds[cantidad].fd = nuevo_socket; // Agrego el nuevo socket al arreglo de fds
            fds[cantidad].events = POLLIN; // Configuro el evento de lectura para el nuevo socket
            fds[cantidad].revents = 0; // Inicializo los eventos de revents en 0
            clientes[cantidad].socket = nuevo_socket; // Inicializo el socket del cliente
            clientes[cantidad].usados = 0; // Inicializo la cantidad de bytes usados en el buffer del cliente
            clientes[cantidad].nombre_usuario[0] = '\0'; // Inicializo el nombre de usuario del cliente
            clientes[cantidad].identificado = 0; // Inicializo el estado de identificación del cliente
            cantidad++; // Incremento la cantidad de clientes conectados
        }
        
        // Manejo la comunicación con los clientes conectados
        for (int i = 1; i < cantidad; i++) {
            if (fds[i].revents & POLLIN) {
                int bytes_leidos = read(fds[i].fd, clientes[i].buffer + clientes[i].usados, BUFFER_SIZE - clientes[i].usados - 1);
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
                        clientes[j] = clientes[j + 1];
                    }
                    cantidad--;
                    i--;
                } else {
                    clientes[i].usados += bytes_leidos;
                    clientes[i].buffer[clientes[i].usados] = '\0'; // Aseguro que el buffer esté terminado en nulo
                    char *fin_mensaje;
                    
                    // Procesar todos los mensajes completos en el buffer del cliente
                    while ((fin_mensaje = strchr(clientes[i].buffer, '\n')) != NULL) {
                        int longitud_mensaje = fin_mensaje - clientes[i].buffer;
                        printf("Mensaje recibido: %.*s\n", longitud_mensaje, clientes[i].buffer);

                        char nombre_usuario[9];

                        // Extraer y validar la identificación del cliente desde el mensaje JSON
                        if (extraer_identificacion(clientes[i].buffer, nombre_usuario, sizeof(nombre_usuario))) {
                            strncpy(clientes[i].nombre_usuario, nombre_usuario, sizeof(clientes[i].nombre_usuario) - 1);
                            clientes[i].nombre_usuario[sizeof(clientes[i].nombre_usuario) - 1] = '\0'; // Aseguro que el nombre de usuario esté terminado en nulo
                            clientes[i].identificado = 1;
                            printf("Cliente identificado como: %s\n", clientes[i].nombre_usuario);

                            char respuesta[256];
                            if (crear_respuesta_identificacion(clientes[i].nombre_usuario, respuesta, sizeof(respuesta))) {
                                enviar_mensaje(clientes[i].socket, respuesta);
                            } else {
                                fprintf(stderr, "Error al crear la respuesta de identificación\n");
                            }
                        }

                        int restante = clientes[i].usados - (longitud_mensaje + 1);
                        memmove(clientes[i].buffer, fin_mensaje + 1, restante);
                        clientes[i].usados = restante;
                        clientes[i].buffer[clientes[i].usados] = '\0'; // Aseguro que el buffer esté terminado en nulo
                    }
                }
            }
        }
    }
    // Cierro todos los sockets y libero la memoria
        for (int i = 1; i < cantidad; i++) {
            close(fds[i].fd);
        }
        close (servidor_socket);
        free(fds);
        free(clientes);
        printf("Servidor cerrado\n");
    return 0;
}