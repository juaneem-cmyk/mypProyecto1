#include <stdio.h>
#include <stdlib.h>
#include "cliente.h"

// Crea un cliente nuevo a partir de un socket ya aceptado y le reserva su buffer de lectura
struct cliente *crear_cliente(int socket, size_t capacidad_buffer) {
    struct cliente *cliente = malloc(sizeof(struct cliente));

    if (cliente == NULL) {
        perror("Error al reservar memoria para el cliente");
        return NULL;
    }
    cliente->socket = socket;
    cliente->estado = ACTIVE;
    cliente->usados = 0;
    cliente->nombre_usuario[0] = '\0';
    cliente->identificado = 0;
    cliente->capacidad_buffer = capacidad_buffer;
    cliente->buffer = malloc(capacidad_buffer);

    if (cliente->buffer == NULL) {
        perror("Error al reservar el buffer del cliente");
        free(cliente);
        return NULL;
    }
    return cliente;
}

// Libera la memoria propia del cliente (buffer y struct)
void destruir_cliente(struct cliente *cliente) {
    if (cliente == NULL) {
        return;
    }
    free(cliente->buffer);
    free(cliente);
}
