#ifndef COMANDOS_H
#define COMANDOS_H

#include <stddef.h>
#include <poll.h>
#include <glib.h>
#include "cliente.h"

struct sala;

// Contiene el estado del servidor que comparten los comandos
struct contexto_servidor {
    struct pollfd *fds;
    struct cliente **clientes;
    size_t *cantidad;
    GHashTable *usuarios;
    GHashTable *salas;
};

typedef int (*manejador_comando)(struct contexto_servidor *contexto, size_t indice);

int manejar_mensaje(struct contexto_servidor *contexto, size_t indice);

void eliminar_cliente(struct contexto_servidor *contexto, size_t indice);

#endif
