#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <glib.h>
#include <signal.h>
#include "Protocolo.h"

/*Voy a seguir un tutorial.
 https://medium.com/@trish07/building-a-simple-tcp-chat-application-in-c-a-step-by-step-tutorial-ed3845607d16 
 Como el tutorial limita los clientes, busque otro tutorial que me sirva de guia
 https://medium.com/@shivambhadani_/understanding-tcp-and-building-our-own-tcp-server-in-c-language-8de9d9de78ef
 */

#define PUERTO 1234
#define BUFFER_SIZE 4096
#define MAX_MENSAJE (1024 * 1024)

// Cerrar adecuadamente el servidor, recomendación de Canek
volatile sig_atomic_t servidor_activo = 1;
void manejar_señal(int señal) {
    servidor_activo = 0;
}

// Enviar respuesta a los clientes conectados
void enviar_mensaje (int socket, const char *mensaje) {
    send(socket, mensaje, strlen(mensaje), 0);
}

enum estado_usuario {
    ACTIVE, AWAY, BUSY
 };

 // Convierte el estado del usuario a texto para enviarlo mediante el protocolo.
const char *estado_a_texto(enum estado_usuario estado) {
    switch (estado) {
        case ACTIVE:
            return "ACTIVE";
        case AWAY:
            return "AWAY";
        case BUSY:
            return "BUSY";
        default:
            return "ACTIVE";
    }
}

struct cliente {
    int socket;
    size_t usados;
    size_t capacidad_buffer;
    char *buffer;
    char nombre_usuario[9];
    int identificado;
    enum estado_usuario estado;
};

// Elimina a un cliente y libera la memoria
void eliminar_cliente(struct pollfd *fds, struct cliente **clientes, int *cantidad, int indice, GHashTable *usuarios){
    if (clientes[indice]->identificado) {
        g_hash_table_remove(usuarios, clientes[indice]->nombre_usuario);
    }
    close(fds[indice].fd);
    free(clientes[indice]->buffer);
    free(clientes[indice]);

    for (int j = indice; j < *cantidad - 1; j++) {
        fds[j] = fds[j + 1];
        clientes[j] = clientes[j + 1];
    }
    (*cantidad)--;
}

// Envía una respuesta de mensaje inválido y desconecta al cliente.
void rechazar_mensaje_invalido(struct pollfd *fds, struct cliente **clientes, int *cantidad, int indice, GHashTable *usuarios) {
    char respuesta[256];

    if (crear_respuesta_invalida("INVALID", respuesta, sizeof(respuesta))) {
        enviar_mensaje(clientes[indice]->socket, respuesta);
    }
    printf("Cliente desconectado: mensaje inválido\n");
    eliminar_cliente(fds, clientes, cantidad, indice, usuarios);
}

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
    struct cliente **clientes = malloc(sizeof(struct cliente *) * capacidad);
    GHashTable *usuarios = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    if (usuarios == NULL){
        fprintf(stderr, "Error al crear la tabla de usuarios\n");
        free(fds);
        free(clientes);
        return 1;
    }
    
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
                struct cliente **temporal_clientes = realloc(clientes, sizeof(struct cliente *) * capacidad);
                
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
            clientes[cantidad] = malloc(sizeof(struct cliente));

            if (clientes[cantidad] == NULL) {
                perror("Error al reservar memoria para el cliente");
                close(nuevo_socket);
                continue;
            }
            clientes[cantidad]->socket = nuevo_socket;
            clientes[cantidad]->estado = ACTIVE;
            clientes[cantidad]->usados = 0;
            clientes[cantidad]->nombre_usuario[0] = '\0';
            clientes[cantidad]->identificado = 0;
            clientes[cantidad]->capacidad_buffer = BUFFER_SIZE;
            clientes[cantidad]->buffer = malloc(BUFFER_SIZE);

            if (clientes[cantidad]->buffer == NULL) {
                perror("Error al reservar el buffer del cliente");
                free(clientes[cantidad]);
                close(nuevo_socket);
                continue;
            }
            cantidad++; // Incremento la cantidad de clientes conectados
        }
        
        // Manejo la comunicación con los clientes conectados
        for (int i = 1; i < cantidad; i++) {
            if (fds[i].revents & POLLIN) {
                char lectura[BUFFER_SIZE];
                int bytes_leidos = read(fds[i].fd, lectura, sizeof(lectura)); 
                if (bytes_leidos < 0) {
                    perror("Error al leer del cliente");
                    continue;
                } 
                if (bytes_leidos == 0) {
                    printf("Cliente desconectado\n");
                    eliminar_cliente(fds, clientes, &cantidad, i, usuarios);
                    i--;
                } else {
                    // Calculo el espacio que necesito para almacenar los datos recibidos
                    size_t espacio_necesario = clientes[i]->usados + bytes_leidos + 1;

                    //Si no tiene espacio, lo amplio
                    if (espacio_necesario > clientes[i]->capacidad_buffer) {
                        size_t nueva_capacidad = clientes[i]->capacidad_buffer * 2;

                        // Duplico la capacidad para que quepan los datos recibidos
                        while (nueva_capacidad < espacio_necesario) {
                            nueva_capacidad *= 2;
                        }
                        // Limito el crecimiento
                        if (nueva_capacidad > MAX_MENSAJE + BUFFER_SIZE + 1) {
                            nueva_capacidad = MAX_MENSAJE + BUFFER_SIZE + 1;
                        }
                        char *nuevo_buffer = realloc(clientes[i]->buffer, nueva_capacidad);

                        if (nuevo_buffer == NULL) {
                            perror("Error al ampliar el buffer del cliente");
                            eliminar_cliente(fds, clientes, &cantidad, i, usuarios);
                            i--;
                            continue;
                        }
                        clientes[i]->buffer = nuevo_buffer;
                        clientes[i]->capacidad_buffer = nueva_capacidad;
                    }
                    // Copio los datos recibidos al buffer del cliente
                    memcpy(clientes[i]->buffer + clientes[i]->usados, lectura, bytes_leidos);
                    clientes[i]->usados += bytes_leidos; // Actualizo los bytes utilizados
                    clientes[i]->buffer[clientes[i]->usados] = '\0'; // Aseguro que el buffer esté terminado en nulo
                
                    char *fin_mensaje;
                    int cliente_eliminado = 0;
                    
                    // Procesar todos los mensajes completos en el buffer del cliente
                    while ((fin_mensaje = strchr(clientes[i]->buffer, '\n')) != NULL) {
                        int longitud_mensaje = fin_mensaje - clientes[i]->buffer;
                        
                        // Verifico que el mensaje no supere el tamaño máximo permitido
                        if (longitud_mensaje > MAX_MENSAJE) {
                            fprintf(stderr, "Mensaje demasiado grande, cliente desconectado\n (˶ᵔ ᵕ ᵔ˶)");
                            eliminar_cliente(fds, clientes, &cantidad, i, usuarios);
                            i--;
                            cliente_eliminado = 1;
                            break;
                        }
                        printf("Mensaje recibido: %.*s\n", longitud_mensaje, clientes[i]->buffer);
                        
                        // Extraer y validar la identificación del cliente desde el mensaje JSON
                        char tipo[32];

                        if (extraer_tipo(clientes[i]->buffer, tipo, sizeof(tipo))) {
                        
                            if (strcmp(tipo, "IDENTIFY") == 0) {
                                char nombre_usuario[9];
                            
                                if (extraer_identificacion(clientes[i]->buffer, nombre_usuario, sizeof(nombre_usuario))) {
                                    char respuesta[256];

                                    if (g_hash_table_contains(usuarios, nombre_usuario)) {
                                    
                                        if (crear_respuesta_identificacion(nombre_usuario, "USER_ALREADY_EXISTS", respuesta, sizeof(respuesta))) {
                                            enviar_mensaje(clientes[i]->socket, respuesta);
                                        }
                                        printf("Cliente desconectado: usuario repetido (%s)\n", nombre_usuario);
                                        eliminar_cliente(fds, clientes, &cantidad, i, usuarios);
                                        i--;
                                        cliente_eliminado = 1;
                                        break;
                                    } else {
                                        strncpy(clientes[i]->nombre_usuario, nombre_usuario, sizeof(clientes[i]->nombre_usuario) - 1);
                                        clientes[i]->nombre_usuario[sizeof(clientes[i]->nombre_usuario) - 1] = '\0';
                                        clientes[i]->identificado = 1;
                                        printf("Cliente identificado como: %s\n", clientes[i]->nombre_usuario);                                        
                                        g_hash_table_insert(usuarios, g_strdup(clientes[i]->nombre_usuario), clientes[i]);

                                        if (crear_respuesta_identificacion(clientes[i]->nombre_usuario, "SUCCESS", respuesta, sizeof(respuesta))) {
                                            enviar_mensaje(clientes[i]->socket, respuesta);
                                        }
                                    }
                                } else {
                                    rechazar_mensaje_invalido(fds, clientes, &cantidad, i, usuarios);
                                    i--;
                                    cliente_eliminado = 1;
                                    break;
                                }

                            // Si el usuario no se identifica
                            } else if (!clientes[i]->identificado) {
                                char respuesta[256];

                                if (crear_respuesta_invalida("NOT_IDENTIFIED", respuesta, sizeof(respuesta))) {
                                    enviar_mensaje(clientes[i]->socket, respuesta);
                                }
                                printf("Cliente desconectado: no estaba identificado\n");
                                eliminar_cliente(fds, clientes, &cantidad, i, usuarios);
                                i--;
                                cliente_eliminado = 1;
                                break;
                            
                            } else if (strcmp(tipo, "STATUS") == 0) {
                                char status[7];
                                // Extraigo y valido el estado solicitado
                                if (extraer_status(clientes[i]->buffer, status, sizeof(status))) {
                                    enum estado_usuario nuevo_estado;

                                    if (strcmp(status, "ACTIVE") == 0) {
                                        nuevo_estado = ACTIVE;
                                    } else if (strcmp(status, "AWAY") == 0) {
                                        nuevo_estado = AWAY;
                                    } else {
                                        nuevo_estado = BUSY;
                                    }

                                    if (clientes[i]->estado != nuevo_estado) {
                                        clientes[i]->estado = nuevo_estado;
                                        char respuesta[256];
                                    
                                        if (crear_nuevo_status(clientes[i]->nombre_usuario, status, respuesta, sizeof(respuesta))) {
                                                            
                                            for (int j = 1; j < cantidad; j++) {
                                                if (j != i && clientes[j]->identificado) {
                                                    enviar_mensaje(clientes[j]->socket, respuesta);
                                                }
                                            }
                                        }
                                    }
                                
                                } else {
                                    rechazar_mensaje_invalido(fds, clientes, &cantidad, i, usuarios);
                                    i--;
                                    cliente_eliminado = 1;
                                    break;
                                }
                            }

                            // Verificamos que el destinatario se encuentra en el diccionario
                            else if (strcmp(tipo, "TEXT") == 0) {
                                char nombre_destino[9];
                                char *texto = NULL;

                                if (extraer_texto(clientes[i]->buffer, nombre_destino, sizeof(nombre_destino), &texto)) {
                                    struct cliente *destinatario = g_hash_table_lookup(usuarios, nombre_destino);

                                    if (destinatario == NULL) {
                                        char respuesta[256];
                                    
                                    } else {
                                    
                                    }
                                    free(texto);
                                
                                } else {
                                    rechazar_mensaje_invalido(fds, clientes, &cantidad, i, usuarios);
                                    i--;
                                    cliente_eliminado = 1;
                                    break;
                                }
                            }
                            // Verifico si el cliente solicitó la lista de usuarios
                            else if (strcmp(tipo, "USERS") == 0) {
                                size_t cantidad_usuarios = g_hash_table_size(usuarios);
                                const char **nombres = malloc(sizeof(char *) * cantidad_usuarios);
                                const char **estados = malloc(sizeof(char *) * cantidad_usuarios);

                                if (nombres == NULL || estados == NULL) {
                                    free(nombres);
                                    free(estados);
                                    continue;
                                }
                            
                                GHashTableIter iter;
                                gpointer clave;
                                gpointer valor;
                                size_t indice_usuario = 0;
                                g_hash_table_iter_init(&iter, usuarios);

                                while (g_hash_table_iter_next(&iter, &clave, &valor)) {
                                    struct cliente *cliente = valor;
                                    nombres[indice_usuario] = cliente->nombre_usuario;
                                    estados[indice_usuario] = estado_a_texto(cliente->estado);
                                    indice_usuario++;
                                }
                                char *respuesta = malloc(MAX_MENSAJE + 2);
                            
                                if (respuesta != NULL) {
                                    if (crear_lista_usuarios(nombres, estados, cantidad_usuarios, respuesta, MAX_MENSAJE + 2)) {
                                        enviar_mensaje(clientes[i]->socket, respuesta);
                                    }
                                    free(respuesta);
                                }
                                free(nombres);
                                free(estados);
                            } else {
                                rechazar_mensaje_invalido(fds, clientes, &cantidad, i, usuarios);
                                i--;
                                cliente_eliminado = 1;
                                break;
                            }
                        } else {
                            rechazar_mensaje_invalido(fds, clientes, &cantidad, i, usuarios);
                            i--;
                            cliente_eliminado = 1;
                            break;
                        }
                        int restante = clientes[i]->usados - (longitud_mensaje + 1);
                        memmove(clientes[i]->buffer, fin_mensaje + 1, restante);
                        clientes[i]->usados = restante;
                        clientes[i]->buffer[clientes[i]->usados] = '\0'; // Aseguro que el buffer esté terminado en nulo
                    }
                    if (cliente_eliminado){
                        continue;
                    }

                    // Verifico si el mensaje incompleto ya superó el tamaño máximo permitido
                    if (clientes[i]->usados > MAX_MENSAJE) {
                        fprintf(stderr, "Mensaje demasiado grande. Cliente desconectado.\n");
                        eliminar_cliente(fds, clientes, &cantidad, i, usuarios);
                        i--;
                        continue;
                    }
                }
            }
        }
    }
    // Cierro todos los sockets y libero la memoria
        for (int i = 1; i < cantidad; i++) {
            close(fds[i].fd);
            free(clientes[i]->buffer);
            free(clientes[i]);
        }
        close (servidor_socket);
        g_hash_table_destroy(usuarios);
        free(fds);
        free(clientes);
        printf("Servidor cerrado\n");
    return 0;
}