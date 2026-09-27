#include <stdlib.h>
#include <string.h>
#include "sala.h"
#include "cliente.h"

// Crea una nueva sala y reserva las referencias de miembros e invitados
struct sala *crear_sala(const char *nombre) {
    struct sala *sala = malloc(sizeof(struct sala));

    if (sala == NULL) {
        return NULL;
    }

    strncpy(sala->nombre, nombre, MAX_NOMBRE_SALA);
    sala->nombre[MAX_NOMBRE_SALA] = '\0';
    sala->miembros = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    sala->invitados = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    if (sala->miembros == NULL || sala->invitados == NULL) {
        g_hash_table_destroy(sala->miembros);
        g_hash_table_destroy(sala->invitados);
        free(sala);
        return NULL;
    }
    return sala;
}

// Libera la memoria asociada a una sala y destruye sus referencias
void destruir_sala(gpointer dato) {
    struct sala *sala = dato;

    if (sala == NULL) {
        return;
    }
    g_hash_table_destroy(sala->miembros);
    g_hash_table_destroy(sala->invitados);
    free(sala);
}

// Agrega un cliente a la lista de miembros de la sala
int sala_agregar_miembro(struct sala *sala, struct cliente *cliente) {
    if (sala == NULL || cliente == NULL) {
        return 0;
    }
    g_hash_table_insert(sala->miembros, g_strdup(cliente->nombre_usuario), cliente);
    return 1;
}

// Agrega un cliente a la lista de invitados de la sala
int sala_agregar_invitado(struct sala *sala, struct cliente *cliente) {
    if (sala == NULL || cliente == NULL) {
        return 0;
    }
    g_hash_table_insert(sala->invitados, g_strdup(cliente->nombre_usuario), cliente);
    return 1;
}
// Mueve un usuario invitado a la lista de miembros de la sala
int sala_unir_miembro(struct sala *sala, struct cliente *cliente) {
    if (sala == NULL || cliente == NULL) {
        return 0;
    }

    if (!sala_es_invitado(sala, cliente->nombre_usuario)) {
        return 0;
    }
    g_hash_table_remove(sala->invitados, cliente->nombre_usuario);
    return sala_agregar_miembro(sala, cliente);
}
// Elimina un usuario de los miembros de la sala
int sala_eliminar_miembro(struct sala *sala, const char *nombre_usuario) {
    if (sala == NULL || nombre_usuario == NULL) {
        return 0;
    }
    return g_hash_table_remove(sala->miembros, nombre_usuario);
}
// Verifica si una sala no tiene miembros
int sala_sin_miembros(struct sala *sala) {
    if (sala == NULL) {
        return 1;
    }
    return g_hash_table_size(sala->miembros) == 0;
}

// Verifica si un usuario ya pertenece a la sala
int sala_es_miembro(struct sala *sala, const char *nombre_usuario) {
    return g_hash_table_contains(sala->miembros, nombre_usuario);
}

// Verifica si un usuario tiene una invitación pendiente en la sala
int sala_es_invitado(struct sala *sala, const char *nombre_usuario) {
    return g_hash_table_contains(sala->invitados, nombre_usuario);
}