#ifndef SALA_H
#define SALA_H

#include <glib.h>

struct cliente;

#define MAX_NOMBRE_SALA 16

struct sala {
    char nombre[MAX_NOMBRE_SALA + 1];
    // Usuarios que ya pertenecen a la sala.
    GHashTable *miembros;
    // Usuarios invitados que todavía no se han unido.
    GHashTable *invitados;
};

struct sala *crear_sala(const char *nombre);
void destruir_sala(struct sala *sala);

#endif