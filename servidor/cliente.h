#ifndef CLIENTE_H
#define CLIENTE_H

#include <stddef.h>

enum estado_usuario {
    ACTIVE, AWAY, BUSY
};

struct cliente {
    int socket;
    size_t usados;
    size_t capacidad_buffer;
    char *buffer;
    char nombre_usuario[9];
    int identificado;
    enum estado_usuario estado;
};

#endif