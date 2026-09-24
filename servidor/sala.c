#include <stdlib.h>
#include <string.h>
#include "sala.h"
#include "cliente.h"

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

void destruir_sala(gpointer dato) {
    struct sala *sala = dato;

    if (sala == NULL) {
        return;
    }
    g_hash_table_destroy(sala->miembros);
    g_hash_table_destroy(sala->invitados);
    free(sala);
}

int sala_agregar_miembro(struct sala *sala, struct cliente *cliente) {
    if (sala == NULL || cliente == NULL) {
        return 0;
    }

    g_hash_table_insert(sala->miembros, g_strdup(cliente->nombre_usuario), cliente);
    return 1;
}

int sala_agregar_invitado(struct sala *sala, struct cliente *cliente) {
    if (sala == NULL || cliente == NULL) {
        return 0;
    }

    g_hash_table_insert(sala->invitados, g_strdup(cliente->nombre_usuario), cliente);
    return 1;
}

int sala_es_miembro(struct sala *sala, const char *nombre_usuario) {
    return g_hash_table_contains(sala->miembros, nombre_usuario);
}

int sala_es_invitado(struct sala *sala, const char *nombre_usuario) {
    return g_hash_table_contains(sala->invitados, nombre_usuario);
}