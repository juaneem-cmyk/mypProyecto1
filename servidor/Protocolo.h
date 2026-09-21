#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <stddef.h>

int extraer_identificacion(const char *mensaje, char *nombre_usuario, size_t usuario_size);

int crear_respuesta_identificacion(const char *nombre_usuario, char *respuesta, size_t respuesta_size);

#endif