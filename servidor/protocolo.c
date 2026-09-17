#include <stdio.h>
#include <string.h>
#include "protocolo.h"

int extraer_identificacion(const char *mensaje, char *nombre_usuario, size_t usuario_size) {
    if (strncmp(mensaje,
                "{\"type\":\"IDENTIFY\"",
                strlen("{\"type\":\"IDENTIFY\"")) != 0) {
        return 0;
    }

    if (sscanf(mensaje,
               "{\"type\":\"IDENTIFY\",\"username\":\"%8[^\"]\"}",
               nombre_usuario) != 1) {
        return 0;
    }

    nombre_usuario[usuario_size - 1] = '\0';

    return 1;
}