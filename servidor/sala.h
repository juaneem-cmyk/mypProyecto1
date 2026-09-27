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
int sala_agregar_miembro(struct sala *sala, struct cliente *cliente);
int sala_agregar_invitado(struct sala *sala, struct cliente *cliente);
int sala_unir_miembro(struct sala *sala, struct cliente *cliente);
int sala_es_miembro(struct sala *sala, const char *nombre_usuario);
int sala_es_invitado(struct sala *sala, const char *nombre_usuario);
int sala_eliminar_miembro(struct sala *sala, const char *nombre_usuario);
int sala_sin_miembros(struct sala *sala);
void destruir_sala(gpointer dato);

#endif