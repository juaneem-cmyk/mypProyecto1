#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "Protocolo.h"
#include "sala.h"
#include "comandos.h"

#define MAX_MENSAJE (1024 * 1024)

// Envía una respuesta al socket del cliente
static void enviar_mensaje(int socket, const char *mensaje) {
    send(socket, mensaje, strlen(mensaje), 0);
}

// Convierte el estado del usuario a texto
static const char *estado_a_texto(enum estado_usuario estado) {
    switch (estado) {
        case ACTIVE:
            return "ACTIVE";
        case AWAY:
            return "AWAY";
        case BUSY:
            return "BUSY";
        default:
            return "ACTIVE";
    }
}

void eliminar_cliente(struct contexto_servidor *contexto, size_t indice) {
    struct cliente **clientes = contexto->clientes;
    size_t *cantidad = contexto->cantidad;

    if (clientes[indice]->identificado) {
        g_hash_table_remove(contexto->usuarios, clientes[indice]->nombre_usuario);
    }
    close(clientes[indice]->socket);
    destruir_cliente(clientes[indice]);

    for (size_t j = indice; j < *cantidad - 1; j++) {
        contexto->fds[j] = contexto->fds[j + 1];
        contexto->clientes[j] = contexto->clientes[j + 1];
    }
    (*cantidad)--;
}

// Envía INVALID y desconecta al cliente por un mensaje inválido
static int rechazar_mensaje_invalido(struct contexto_servidor *contexto, size_t indice) {
    char respuesta[256];

    if (crear_respuesta_invalida("INVALID", respuesta, sizeof(respuesta))) {
        enviar_mensaje(contexto->clientes[indice]->socket, respuesta);
    }
    printf("Cliente desconectado: mensaje inválido\n");
    eliminar_cliente(contexto, indice);
    return 1;
}

// Procesa IDENTIFY
static int manejar_identify(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char nombre_usuario[9];

    if (!extraer_identificacion(cliente->buffer, nombre_usuario, sizeof(nombre_usuario))) {
        return rechazar_mensaje_invalido(contexto, indice);
    }
    char respuesta[256];

    if (g_hash_table_contains(contexto->usuarios, nombre_usuario)) {
        if (crear_respuesta_identificacion(nombre_usuario, "USER_ALREADY_EXISTS", respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
        printf("Cliente desconectado: usuario repetido (%s)\n", nombre_usuario);
        eliminar_cliente(contexto, indice);
        return 1;
    }
    strncpy(cliente->nombre_usuario, nombre_usuario, sizeof(cliente->nombre_usuario) - 1);
    cliente->nombre_usuario[sizeof(cliente->nombre_usuario) - 1] = '\0';
    cliente->identificado = 1;
    printf("Cliente identificado como: %s\n", cliente->nombre_usuario);
    g_hash_table_insert(contexto->usuarios, g_strdup(cliente->nombre_usuario), cliente);

    if (crear_respuesta_identificacion(cliente->nombre_usuario, "SUCCESS", respuesta, sizeof(respuesta))) {
        enviar_mensaje(cliente->socket, respuesta);
        char notificacion_usuario[256];

        if (crear_nuevo_usuario(cliente->nombre_usuario, notificacion_usuario, sizeof(notificacion_usuario))) {
            for (size_t j = 1; j < *contexto->cantidad; j++) {
                if (j != indice && contexto->clientes[j]->identificado) {
                    enviar_mensaje(contexto->clientes[j]->socket, notificacion_usuario);
                }
            }
        }
    }
    return 0;
}

// Procesa NEW_ROOM
static int manejar_new_room(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char nombre_sala[MAX_NOMBRE_SALA + 1];

    if (!extraer_nombre_sala(cliente->buffer, nombre_sala, sizeof(nombre_sala))) {
        return rechazar_mensaje_invalido(contexto, indice);
    }
    struct sala *sala_existente = g_hash_table_lookup(contexto->salas, nombre_sala);

    if (sala_existente != NULL) {
        char respuesta[256];
        if (crear_respuesta_operacion("NEW_ROOM", "ROOM_ALREADY_EXISTS", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
        return 0;
    }

    struct sala *nueva_sala = crear_sala(nombre_sala);
    if (nueva_sala == NULL) {
        fprintf(stderr, "Error al crear la sala\n");
        return 0;
    }

    if (sala_agregar_miembro(nueva_sala, cliente)) {
        g_hash_table_insert(contexto->salas, g_strdup(nombre_sala), nueva_sala);
        char respuesta[256];

        if (crear_respuesta_operacion("NEW_ROOM", "SUCCESS", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
    } else {
        destruir_sala(nueva_sala);
        fprintf(stderr, "Error al agregar creador a la sala\n");
    }
    return 0;
}

// Procesa INVITE
static int manejar_invite(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char nombre_sala[MAX_NOMBRE_SALA + 1];
    char **nombres_usuarios = NULL;
    size_t cantidad_usuarios = 0;

    if (!extraer_invitacion(cliente->buffer, nombre_sala, sizeof(nombre_sala), &nombres_usuarios, &cantidad_usuarios)) {
        return rechazar_mensaje_invalido(contexto, indice);
    }
    struct sala *sala = g_hash_table_lookup(contexto->salas, nombre_sala);

    if (sala == NULL) {
        char respuesta[256];
        if (crear_respuesta_operacion("INVITE", "NO_SUCH_ROOM", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
    } else if (!sala_es_miembro(sala, cliente->nombre_usuario)) {
        printf("El usuario %s no pertenece a la sala %s\n", cliente->nombre_usuario, nombre_sala);
    } else {
        int error = 0;

        for (size_t j = 0; j < cantidad_usuarios; j++) {
            struct cliente *invitado = g_hash_table_lookup(contexto->usuarios, nombres_usuarios[j]);

            if (invitado == NULL) {
                char respuesta[256];
                if (crear_respuesta_operacion("INVITE", "NO_SUCH_USER", nombres_usuarios[j], respuesta, sizeof(respuesta))) {
                    enviar_mensaje(cliente->socket, respuesta);
                }
                error = 1;
                break;
            }
        }

        if (!error) {
            for (size_t j = 0; j < cantidad_usuarios; j++) {
                struct cliente *invitado = g_hash_table_lookup(contexto->usuarios, nombres_usuarios[j]);

                if (sala_es_miembro(sala, nombres_usuarios[j]) || sala_es_invitado(sala, nombres_usuarios[j])) {
                    continue;
                }
                sala_agregar_invitado(sala, invitado);
                char respuesta[256];

                if (crear_invitacion(cliente->nombre_usuario, nombre_sala, respuesta, sizeof(respuesta))) {
                    enviar_mensaje(invitado->socket, respuesta);
                }
            }
        }
    }
    for (size_t j = 0; j < cantidad_usuarios; j++) {
        free(nombres_usuarios[j]);
    }
    free(nombres_usuarios);
    return 0;
}

// Procesa JOIN_ROOM
static int manejar_join_room(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char nombre_sala[MAX_NOMBRE_SALA + 1];

    if (!extraer_union_sala(cliente->buffer, nombre_sala, sizeof(nombre_sala))) {
        return rechazar_mensaje_invalido(contexto, indice);
    }
    struct sala *sala = g_hash_table_lookup(contexto->salas, nombre_sala);

    if (sala == NULL) {
        char respuesta[256];
        if (crear_respuesta_operacion("JOIN_ROOM", "NO_SUCH_ROOM", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
    } else if (!sala_es_invitado(sala, cliente->nombre_usuario)) {
        char respuesta[256];
        if (crear_respuesta_operacion("JOIN_ROOM", "NOT_INVITED", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
    } else if (sala_unir_miembro(sala, cliente)) {
        char respuesta[256];
        if (crear_respuesta_operacion("JOIN_ROOM", "SUCCESS", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
        char notificacion[256];

        if (crear_usuario_unido(cliente->nombre_usuario, nombre_sala, notificacion, sizeof(notificacion))) {
            GHashTableIter iter;
            gpointer clave;
            gpointer valor;
            g_hash_table_iter_init(&iter, sala->miembros);

            while (g_hash_table_iter_next(&iter, &clave, &valor)) {
                struct cliente *miembro = valor;
                enviar_mensaje(miembro->socket, notificacion);
            }
        }
    }
    return 0;
}

// Procesa ROOM_USERS
static int manejar_room_users(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char nombre_sala[MAX_NOMBRE_SALA + 1];

    if (!extraer_usuarios_sala(cliente->buffer, nombre_sala, sizeof(nombre_sala))) {
        return rechazar_mensaje_invalido(contexto, indice);
    }
    struct sala *sala = g_hash_table_lookup(contexto->salas, nombre_sala);

    if (sala == NULL) {
        char respuesta[256];
        if (crear_respuesta_operacion("ROOM_USERS", "NO_SUCH_ROOM", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
        return 0;
    }

    if (!sala_es_miembro(sala, cliente->nombre_usuario)) {
        char respuesta[256];
        if (crear_respuesta_operacion("ROOM_USERS", "NOT_JOINED", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
        return 0;
    }
    size_t cantidad_miembros = g_hash_table_size(sala->miembros);
    const char **nombres = malloc(sizeof(char *) * cantidad_miembros);
    const char **estados = malloc(sizeof(char *) * cantidad_miembros);

    if (nombres == NULL || estados == NULL) {
        free(nombres);
        free(estados);
        return 0;
    }
    GHashTableIter iter;
    gpointer clave;
    gpointer valor;
    size_t indice_miembro = 0;
    g_hash_table_iter_init(&iter, sala->miembros);

    while (g_hash_table_iter_next(&iter, &clave, &valor)) {
        struct cliente *miembro = valor;
        nombres[indice_miembro] = miembro->nombre_usuario;
        estados[indice_miembro] = estado_a_texto(miembro->estado);
        indice_miembro++;
    }
    char *respuesta = malloc(MAX_MENSAJE + 2);

    if (respuesta != NULL) {
        if (crear_lista_usuarios_sala(nombre_sala, nombres, estados, cantidad_miembros, respuesta, MAX_MENSAJE + 2)) {
            enviar_mensaje(cliente->socket, respuesta);
        }
        free(respuesta);
    }
    free(nombres);
    free(estados);
    return 0;
}

// Procesa ROOM_TEXT
static int manejar_room_text(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char nombre_sala[MAX_NOMBRE_SALA + 1];
    char *texto = NULL;

    if (!extraer_texto_sala(cliente->buffer, nombre_sala, sizeof(nombre_sala), &texto)) {
        return rechazar_mensaje_invalido(contexto, indice);
    }
    struct sala *sala = g_hash_table_lookup(contexto->salas, nombre_sala);

    if (sala == NULL) {
        char respuesta[256];
        if (crear_respuesta_operacion("ROOM_TEXT", "NO_SUCH_ROOM", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
    } else if (!sala_es_miembro(sala, cliente->nombre_usuario)) {
        char respuesta[256];
        if (crear_respuesta_operacion("ROOM_TEXT", "NOT_JOINED", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
    } else {
        char respuesta[1024];

        if (crear_texto_sala_desde(cliente->nombre_usuario, nombre_sala, texto, respuesta, sizeof(respuesta))) {
            GHashTableIter iter;
            gpointer clave;
            gpointer valor;
            g_hash_table_iter_init(&iter, sala->miembros);

            while (g_hash_table_iter_next(&iter, &clave, &valor)) {
                struct cliente *miembro = valor;
                if (miembro != cliente) {
                    enviar_mensaje(miembro->socket, respuesta);
                }
            }
        }
    }
    free(texto);
    return 0;
}

// Procesa LEAVE_ROOM
static int manejar_leave_room(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char nombre_sala[MAX_NOMBRE_SALA + 1];

    if (!extraer_salida_sala(cliente->buffer, nombre_sala, sizeof(nombre_sala))) {
        return rechazar_mensaje_invalido(contexto, indice);
    }
    struct sala *sala = g_hash_table_lookup(contexto->salas, nombre_sala);

    if (sala == NULL) {
        char respuesta[256];
        if (crear_respuesta_operacion("LEAVE_ROOM", "NO_SUCH_ROOM", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
    } else if (!sala_es_miembro(sala, cliente->nombre_usuario)) {
        char respuesta[256];
        if (crear_respuesta_operacion("LEAVE_ROOM", "NOT_JOINED", nombre_sala, respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
    } else {
        char notificacion[256];

        if (crear_usuario_salio(cliente->nombre_usuario, nombre_sala, notificacion, sizeof(notificacion))) {
            GHashTableIter iter;
            gpointer clave;
            gpointer valor;
            g_hash_table_iter_init(&iter, sala->miembros);

            while (g_hash_table_iter_next(&iter, &clave, &valor)) {
                struct cliente *miembro = valor;
                if (miembro != cliente) {
                    enviar_mensaje(miembro->socket, notificacion);
                }
            }
        }
        sala_eliminar_miembro(sala, cliente->nombre_usuario);
        if (sala_sin_miembros(sala)) {
            g_hash_table_remove(contexto->salas, nombre_sala);
        }
    }
    return 0;
}

// Procesa DISCONNECT y elimina al cliente
static int manejar_disconnect(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char respuesta_desconexion[256];

    if (crear_usuario_desconectado(cliente->nombre_usuario, respuesta_desconexion, sizeof(respuesta_desconexion))) {
        for (size_t j = 1; j < *contexto->cantidad; j++) {
            if (j != indice && contexto->clientes[j]->identificado) {
                enviar_mensaje(contexto->clientes[j]->socket, respuesta_desconexion);
            }
        }
    }
    GHashTableIter iter_salas;
    gpointer clave_sala;
    gpointer valor_sala;
    g_hash_table_iter_init(&iter_salas, contexto->salas);

    while (g_hash_table_iter_next(&iter_salas, &clave_sala, &valor_sala)) {
        struct sala *sala = valor_sala;

        if (!sala_es_miembro(sala, cliente->nombre_usuario)) {
            continue;
        }
        char notificacion[256];

        if (crear_usuario_salio(cliente->nombre_usuario, sala->nombre, notificacion, sizeof(notificacion))) {
            GHashTableIter iter_miembros;
            gpointer clave_miembro;
            gpointer valor_miembro;
            g_hash_table_iter_init(&iter_miembros, sala->miembros);

            while (g_hash_table_iter_next(&iter_miembros, &clave_miembro, &valor_miembro)) {
                struct cliente *miembro = valor_miembro;
                if (miembro != cliente) {
                    enviar_mensaje(miembro->socket, notificacion);
                }
            }
        }
        sala_eliminar_miembro(sala, cliente->nombre_usuario);

        if (sala_sin_miembros(sala)) {
            g_hash_table_iter_remove(&iter_salas);
        }
    }
    printf("Cliente desconectado: %s\n", cliente->nombre_usuario);
    eliminar_cliente(contexto, indice);
    return 1;
}

// Procesa STATUS
static int manejar_status(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char status[7];

    if (!extraer_status(cliente->buffer, status, sizeof(status))) {
        return rechazar_mensaje_invalido(contexto, indice);
    }
    enum estado_usuario nuevo_estado;

    if (strcmp(status, "ACTIVE") == 0) {
        nuevo_estado = ACTIVE;
    } else if (strcmp(status, "AWAY") == 0) {
        nuevo_estado = AWAY;
    } else {
        nuevo_estado = BUSY;
    }

    if (cliente->estado != nuevo_estado) {
        cliente->estado = nuevo_estado;
        char respuesta[256];

        if (crear_nuevo_status(cliente->nombre_usuario, status, respuesta, sizeof(respuesta))) {
            for (size_t j = 1; j < *contexto->cantidad; j++) {
                if (j != indice && contexto->clientes[j]->identificado) {
                    enviar_mensaje(contexto->clientes[j]->socket, respuesta);
                }
            }
        }
    }
    return 0;
}

// Procesa TEXT
static int manejar_text(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char nombre_destino[9];
    char *texto = NULL;

    if (!extraer_texto(cliente->buffer, nombre_destino, sizeof(nombre_destino), &texto)) {
        return rechazar_mensaje_invalido(contexto, indice);
    }
    struct cliente *destinatario = g_hash_table_lookup(contexto->usuarios, nombre_destino);

    if (destinatario == NULL) {
        char respuesta[256];
        if (crear_respuesta_texto(nombre_destino, "NO_SUCH_USER", respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
    } else {
        char respuesta[256];
        if (crear_texto_desde(cliente->nombre_usuario, texto, respuesta, sizeof(respuesta))) {
            enviar_mensaje(destinatario->socket, respuesta);
        }
    }
    free(texto);
    return 0;
}

// Procesa PUBLIC_TEXT
static int manejar_public_text(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char *texto = NULL;

    if (!extraer_texto_publico(cliente->buffer, &texto)) {
        return rechazar_mensaje_invalido(contexto, indice);
    }
    char *respuesta = malloc(MAX_MENSAJE + 2);

    if (respuesta != NULL) {
        if (crear_texto_publico_desde(cliente->nombre_usuario, texto, respuesta, MAX_MENSAJE + 2)) {
            for (size_t j = 1; j < *contexto->cantidad; j++) {
                if (j != indice && contexto->clientes[j]->identificado) {
                    enviar_mensaje(contexto->clientes[j]->socket, respuesta);
                }
            }
        }
        free(respuesta);
    }
    free(texto);
    return 0;
}

// Procesa USERS
static int manejar_users(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    size_t cantidad_usuarios = g_hash_table_size(contexto->usuarios);
    const char **nombres = malloc(sizeof(char *) * cantidad_usuarios);
    const char **estados = malloc(sizeof(char *) * cantidad_usuarios);

    if (nombres == NULL || estados == NULL) {
        free(nombres);
        free(estados);
        return 0;
    }
    GHashTableIter iter;
    gpointer clave;
    gpointer valor;
    size_t indice_usuario = 0;
    g_hash_table_iter_init(&iter, contexto->usuarios);

    while (g_hash_table_iter_next(&iter, &clave, &valor)) {
        struct cliente *usuario = valor;
        nombres[indice_usuario] = usuario->nombre_usuario;
        estados[indice_usuario] = estado_a_texto(usuario->estado);
        indice_usuario++;
    }
    char *respuesta = malloc(MAX_MENSAJE + 2);

    if (respuesta != NULL) {
        if (crear_lista_usuarios(nombres, estados, cantidad_usuarios, respuesta, MAX_MENSAJE + 2)) {
            enviar_mensaje(cliente->socket, respuesta);
        }
        free(respuesta);
    }
    free(nombres);
    free(estados);
    return 0;
}

// Relaciona cada tipo de mensaje con su operación
static const struct {const char *tipo; manejador_comando manejar;} comandos[] = {
    {"IDENTIFY", manejar_identify},
    {"NEW_ROOM", manejar_new_room},
    {"INVITE", manejar_invite},
    {"JOIN_ROOM", manejar_join_room},
    {"ROOM_USERS", manejar_room_users},
    {"ROOM_TEXT", manejar_room_text},
    {"LEAVE_ROOM", manejar_leave_room},
    {"DISCONNECT", manejar_disconnect},
    {"STATUS", manejar_status},
    {"TEXT", manejar_text},
    {"PUBLIC_TEXT", manejar_public_text},
    {"USERS", manejar_users}
};

int manejar_mensaje(struct contexto_servidor *contexto, size_t indice) {
    struct cliente *cliente = contexto->clientes[indice];
    char tipo[32];

    if (!extraer_tipo(cliente->buffer, tipo, sizeof(tipo))) {
        return rechazar_mensaje_invalido(contexto, indice);
    }

    if (strcmp(tipo, "IDENTIFY") != 0 && !cliente->identificado) {
        char respuesta[256];

        if (crear_respuesta_invalida("NOT_IDENTIFIED", respuesta, sizeof(respuesta))) {
            enviar_mensaje(cliente->socket, respuesta);
        }
        printf("Cliente desconectado: no estaba identificado\n");
        eliminar_cliente(contexto, indice);
        return 1;
    }
    size_t cantidad_comandos = sizeof(comandos) / sizeof(comandos[0]);

    for (size_t i = 0; i < cantidad_comandos; i++) {
        if (strcmp(tipo, comandos[i].tipo) == 0) {
            return comandos[i].manejar(contexto, indice);
        }
    }
    return rechazar_mensaje_invalido(contexto, indice);
}
