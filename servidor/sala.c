#include <stdlib.h>
#include <string.h>
#include "sala.h"

struct sala *crear_sala(const char *nombre) {
    struct sala *sala = malloc(sizeof(struct sala));

    if (sala == NULL) {
        return NULL;
    }

    strncpy(sala->nombre, nombre, MAX_NOMBRE_SALA);
    sala->nombre[MAX_NOMBRE_SALA] = '\0';
    sala->miembros = g_hash_table_new(g_str_hash, g_str_equal);
    sala->invitados = g_hash_table_new(g_str_hash, g_str_equal);

    if (sala->miembros == NULL || sala->invitados == NULL) {
        g_hash_table_destroy(sala->miembros);
        g_hash_table_destroy(sala->invitados);
        free(sala);
        return NULL;
    }
    return sala;
}

void destruir_sala(struct sala *sala) {
    if (sala == NULL) {
        return;
    }
    g_hash_table_destroy(sala->miembros);
    g_hash_table_destroy(sala->invitados);
    free(sala);
}