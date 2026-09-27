#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <stddef.h>

int extraer_identificacion(const char *mensaje, char *nombre_usuario, size_t usuario_size);

int extraer_texto(const char *mensaje, char *nombre_usuario, size_t usuario_size, char **texto);

int extraer_tipo(const char *mensaje, char *tipo, size_t tipo_size);

int extraer_status(const char *mensaje, char *status, size_t status_size);

int extraer_nombre_sala(const char *mensaje, char *nombre_sala, size_t sala_size);

int extraer_invitacion(const char *mensaje, char *nombre_sala, size_t sala_size, char ***usuarios, size_t *cantidad_usuarios);

int extraer_union_sala(const char *mensaje, char *nombre_sala, size_t sala_size);

int extraer_usuarios_sala(const char *mensaje, char *nombre_sala, size_t sala_size);

int extraer_texto_sala(const char *mensaje, char *nombre_sala, size_t sala_size, char **texto);

int extraer_salida_sala(const char *mensaje, char *nombre_sala, size_t sala_size);

int extraer_texto_publico(const char *mensaje, char **texto);

int crear_texto_publico_desde(const char *nombre_usuario, const char *texto, char *respuesta, size_t respuesta_size);

int crear_usuario_salio(const char *nombre_usuario, const char *nombre_sala, char *respuesta, size_t respuesta_size);

int crear_texto_sala_desde(const char *nombre_usuario, const char *nombre_sala, const char *texto, char *respuesta, size_t respuesta_size);

int crear_lista_usuarios_sala(const char *nombre_sala, const char *nombres[], const char *estados[], size_t cantidad, char *respuesta, size_t respuesta_size);

int crear_usuario_unido(const char *nombre_usuario, const char *nombre_sala, char *respuesta, size_t respuesta_size);

int crear_respuesta_identificacion(const char *nombre_usuario, const char *resultado, char *respuesta, size_t respuesta_size);

int crear_respuesta_invalida(const char *resultado, char *respuesta, size_t respuesta_size);

int crear_texto_desde(const char *nombre_usuario, const char *texto, char *respuesta, size_t respuesta_size);

int crear_respuesta_texto(const char *nombre_usuario, const char *resultado, char *respuesta, size_t respuesta_size);

int crear_lista_usuarios(const char *nombres[], const char *estados[], size_t cantidad, char *respuesta, size_t respuesta_size);

int crear_nuevo_status(const char *nombre_usuario, const char *status, char *respuesta, size_t respuesta_size);

int crear_respuesta_sala(const char *operacion, const char *resultado, const char *extra, char *respuesta, size_t respuesta_size);

int crear_invitacion(const char *nombre_usuario, const char *nombre_sala, char *respuesta, size_t respuesta_size);

int crear_respuesta_operacion(const char *operacion, const char *resultado, const char *extra, char *respuesta, size_t respuesta_size);

int crear_nuevo_usuario(const char *nombre_usuario, char *respuesta, size_t respuesta_size);

int crear_usuario_desconectado(const char *nombre_usuario, char *respuesta, size_t respuesta_size);

#endif