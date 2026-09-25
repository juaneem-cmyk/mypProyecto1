#include <string.h>
#include <stdlib.h>
#include <json-c/json.h>
#include "Protocolo.h"

/* Me dí cuenta de mi error al no usar json-c, pero busqué lugares para guiarme
    https://github.com/JimmyWorks/TCP-Chat-App.git
    https://github.com/json-c/json-c
    Tuve que refactorizar varias cosas */

// Extrae el tipo de mensaje que mandó el cliente
int extraer_tipo(const char *mensaje, char *tipo, size_t tipo_size) {
    struct json_object *objeto;
    struct json_object *tipo_json;
    const char *texto_tipo;

    objeto = json_tokener_parse(mensaje);

    if (objeto == NULL ||
        !json_object_is_type(objeto, json_type_object) ||
        !json_object_object_get_ex(objeto, "type", &tipo_json) ||
        !json_object_is_type(tipo_json, json_type_string)) {
        if (objeto != NULL) {
            json_object_put(objeto);
        }
        return 0;
    }

    texto_tipo = json_object_get_string(tipo_json);

    if (texto_tipo == NULL || strlen(texto_tipo) >= tipo_size) {
        json_object_put(objeto);
        return 0;
    }

    strcpy(tipo, texto_tipo);

    json_object_put(objeto);
    return 1;
}

// Extrae y valida el estado solicitado por el cliente
int extraer_status(const char *mensaje, char *status, size_t status_size) {
    struct json_object *objeto;
    struct json_object *tipo;
    struct json_object *estado;
    const char *texto_estado;

    objeto = json_tokener_parse(mensaje);

    if (objeto == NULL) {
        return 0;
    }

    if (!json_object_is_type(objeto, json_type_object)) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "type", &tipo)) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_is_type(tipo, json_type_string) || strcmp(json_object_get_string(tipo), "STATUS") != 0) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "status", &estado)) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_is_type(estado, json_type_string)) {
        json_object_put(objeto);
        return 0;
    }
    texto_estado = json_object_get_string(estado);

    // Verificar que el estado sea uno de los permitidos
    if (strcmp(texto_estado, "ACTIVE") != 0 && strcmp(texto_estado, "AWAY") != 0 && strcmp(texto_estado, "BUSY") != 0) {
        json_object_put(objeto);
        return 0;
    }

    if (strlen(texto_estado) >= status_size) {
        json_object_put(objeto);
        return 0;
    }
    strcpy(status, texto_estado);
    json_object_put(objeto);
    return 1;
}

// Crea el mensaje NEW_STATUS para notificar a los demás clientes
int crear_nuevo_status(const char *nombre_usuario, const char *status, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    const char *texto_json;
    size_t longitud;
    objeto = json_object_new_object();

    if (objeto == NULL) {
        return 0;
    }

    json_object_object_add(objeto, "type", json_object_new_string("NEW_STATUS"));
    json_object_object_add(objeto, "username", json_object_new_string(nombre_usuario));
    json_object_object_add(objeto, "status", json_object_new_string(status));
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) {
        json_object_put(objeto);
        return 0;
    }
    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n';
    respuesta[longitud + 1] = '\0';
    json_object_put(objeto);
    return 1;
}

// Crea una respuesta para indicar que el mensaje recibido no es válido
int crear_respuesta_invalida(const char *resultado, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    const char *texto_json;
    size_t longitud;

    objeto = json_object_new_object();

    if (objeto == NULL) {
        return 0;
    }

    json_object_object_add(objeto, "type", json_object_new_string("RESPONSE"));
    json_object_object_add(objeto, "operation", json_object_new_string("INVALID"));
    json_object_object_add(objeto, "result", json_object_new_string(resultado));
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) {
        json_object_put(objeto);
        return 0;
    }

    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n';
    respuesta[longitud + 1] = '\0';
    json_object_put(objeto);
    return 1;
}

// Extrae y valida la identificación del cliente desde el mensaje JSON.
int extraer_identificacion(const char *mensaje, char *nombre_usuario, size_t usuario_size) {
    struct json_object *objeto;
    struct json_object *tipo;
    struct json_object *usuario;

    // Parsear el mensaje JSON
    objeto = json_tokener_parse(mensaje);
    if (objeto == NULL) {
        return 0; // Error al parsear JSON
    }

    // Verificar que el JSON recibido sea un objeto
    if (!json_object_is_type(objeto, json_type_object)) {
        json_object_put(objeto); // Liberar memoria del objeto JSON
        return 0; // Tipo incorrecto
    }

    // Extraer el campo "type" y verificar que sea "IDENTIFY"
    if (!json_object_object_get_ex(objeto, "type", &tipo)) {
        json_object_put(objeto); 
        return 0; // No se encontró el campo type
    }

    // Verificar que el tipo sea "IDENTIFY"
    if (strcmp(json_object_get_string(tipo), "IDENTIFY") != 0) {
        json_object_put(objeto);
        return 0; // Tipo incorrecto
    }
    
    // Extraer el nombre de usuario
    if (!json_object_object_get_ex(objeto, "username", &usuario)) {
        json_object_put(objeto); 
        return 0; // No se encontró el campo username
    }

    // Verificar que el campo username sea una cadena de caracteres
    if (!json_object_is_type(usuario, json_type_string)) {
        json_object_put(objeto); 
        return 0; // Tipo incorrecto para username
    }

    const char *nombre = json_object_get_string(usuario);
    if (nombre == NULL || strlen(nombre) >= usuario_size) {
        json_object_put(objeto);
        return 0; // Nombre de usuario inválido o demasiado largo
    }

    strncpy(nombre_usuario, nombre, usuario_size - 1);
    nombre_usuario[usuario_size - 1] = '\0'; // Asegurar terminación nula
    json_object_put(objeto); 
    return 1; 
}

// Extrae del mensaje JSON el nombre del usuario destinatario y el texto enviado
int extraer_texto(const char *mensaje, char *nombre_usuario, size_t usuario_size, char **texto) {
    struct json_object *objeto;
    struct json_object *tipo;
    struct json_object *usuario;
    struct json_object *texto_json;
    const char *nombre;
    const char *contenido;
    *texto = NULL;
    objeto = json_tokener_parse(mensaje);

    if (objeto == NULL || !json_object_is_type(objeto, json_type_object)) {
        if (objeto != NULL) {
            json_object_put(objeto);
        }
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "type", &tipo) || !json_object_is_type(tipo, json_type_string) || strcmp(json_object_get_string(tipo), "TEXT") != 0) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "username", &usuario) || !json_object_is_type(usuario, json_type_string)) {
        json_object_put(objeto);
        return 0;
    }

    nombre = json_object_get_string(usuario);

    if (nombre == NULL || strlen(nombre) >= usuario_size) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "text", &texto_json) || !json_object_is_type(texto_json, json_type_string)) {
        json_object_put(objeto);
        return 0;
    }

    contenido = json_object_get_string(texto_json);

    if (contenido == NULL) {
        json_object_put(objeto);
        return 0;
    }
    strncpy(nombre_usuario, nombre, usuario_size - 1);
    nombre_usuario[usuario_size - 1] = '\0';
    *texto = malloc(strlen(contenido) + 1);

    if (*texto == NULL) {
        json_object_put(objeto);
        return 0;
    }
    strcpy(*texto, contenido);
    json_object_put(objeto);
    return 1;
}

// Crea una respuesta JSON para la identificación del cliente. (esta parte la estaba escribiendo cuando me di cuenta de mi error y comencé a refactorizar)
int crear_respuesta_identificacion(const char *nombre_usuario,const char *resultado, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    const char *texto_json;
    size_t longitud;

    // Crear un objeto JSON para la respuesta
    objeto = json_object_new_object();
    if (objeto == NULL) {
        return 0; // Error al crear el objeto JSON
    }
    json_object_object_add(objeto, "type", json_object_new_string("RESPONSE"));
    json_object_object_add(objeto, "operation", json_object_new_string("IDENTIFY"));
    json_object_object_add(objeto, "result", json_object_new_string(resultado));
    json_object_object_add(objeto, "extra", json_object_new_string(nombre_usuario));
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) { // +2 para el salto de línea y el terminador nulo
        json_object_put(objeto); 
        return 0; // Buffer insuficiente
    }
    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n'; // Agregar salto de línea al final
    respuesta[longitud + 1] = '\0'; // Asegurar terminación nula
    json_object_put(objeto);
    return 1;
}

// Crea una respuesta JSON para la operación TEXT
int crear_respuesta_texto(const char *nombre_usuario, const char *resultado, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    const char *texto_json;
    size_t longitud;
    objeto = json_object_new_object();

    if (objeto == NULL) {
        return 0;
    }
    json_object_object_add(objeto, "type", json_object_new_string("RESPONSE"));
    json_object_object_add(objeto, "operation", json_object_new_string("TEXT"));
    json_object_object_add(objeto, "result", json_object_new_string(resultado));
    json_object_object_add(objeto, "extra", json_object_new_string(nombre_usuario));
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) {
        json_object_put(objeto);
        return 0;
    }
    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n';
    respuesta[longitud + 1] = '\0';
    json_object_put(objeto);
    return 1;
}

// Crea el mensaje JSON que se enviará al destinatario de un mensaje privado.
int crear_texto_desde(const char *nombre_usuario, const char *texto, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    const char *texto_json;
    size_t longitud;
    objeto = json_object_new_object();

    if (objeto == NULL) {
        return 0;
    }

    json_object_object_add(objeto, "type", json_object_new_string("TEXT_FROM"));
    json_object_object_add(objeto, "username", json_object_new_string(nombre_usuario));
    json_object_object_add(objeto, "text", json_object_new_string(texto));
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) {
        json_object_put(objeto);
        return 0;
    }
    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n';
    respuesta[longitud + 1] = '\0';
    json_object_put(objeto);
    return 1;
}

// Crea el mensaje JSON con la lista de usuarios y sus estados
int crear_lista_usuarios(const char *nombres[], const char *estados[], size_t cantidad, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    struct json_object *usuarios;
    const char *texto_json;
    size_t longitud;
    objeto = json_object_new_object();
    usuarios = json_object_new_object();

    if (objeto == NULL || usuarios == NULL) {
        if (objeto != NULL) {
            json_object_put(objeto);
        }

        if (usuarios != NULL) {
            json_object_put(usuarios);
        }
        return 0;
    }
    json_object_object_add(objeto, "type", json_object_new_string("USER_LIST"));

    for (size_t i = 0; i < cantidad; i++) {
        json_object_object_add(usuarios, nombres[i], json_object_new_string(estados[i]));
    }
    json_object_object_add(objeto, "users", usuarios);
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) {
        json_object_put(objeto);
        return 0;
    }
    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n';
    respuesta[longitud + 1] = '\0';
    json_object_put(objeto);
    return 1;
}

// Extrae y valida el nombre de una sala para NEW_ROOM
int extraer_nombre_sala(const char *mensaje, char *nombre_sala, size_t sala_size) {
    struct json_object *objeto;
    struct json_object *tipo;
    struct json_object *sala;
    const char *texto_sala;
    objeto = json_tokener_parse(mensaje);

    if (objeto == NULL) {
        return 0;
    }

    if (!json_object_is_type(objeto, json_type_object)) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "type", &tipo) || !json_object_is_type(tipo, json_type_string) || strcmp(json_object_get_string(tipo), "NEW_ROOM") != 0) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "roomname", &sala) || !json_object_is_type(sala, json_type_string)) {
        json_object_put(objeto);
        return 0;
    }
    texto_sala = json_object_get_string(sala);

    if (texto_sala == NULL || strlen(texto_sala) == 0 || strlen(texto_sala) >= sala_size) {
        json_object_put(objeto);
        return 0;
    }
    strcpy(nombre_sala, texto_sala);
    json_object_put(objeto);
    return 1;
}

// Crea una respuesta JSON para una operación relacionada con salas
int crear_respuesta_sala(const char *operacion, const char *resultado, const char *extra, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    const char *texto_json;
    size_t longitud;
    objeto = json_object_new_object();

    if (objeto == NULL) {
        return 0;
    }
    json_object_object_add(objeto, "type", json_object_new_string("RESPONSE"));
    json_object_object_add(objeto, "operation", json_object_new_string(operacion));
    json_object_object_add(objeto, "result", json_object_new_string(resultado));

    if (extra != NULL) {
        json_object_object_add(objeto, "extra", json_object_new_string(extra));
    }
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) {
        json_object_put(objeto);
        return 0;
    }
    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n';
    respuesta[longitud + 1] = '\0';
    json_object_put(objeto);
    return 1;
}

int crear_respuesta_operacion(const char *operacion, const char *resultado, const char *extra, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    const char *texto_json;
    size_t longitud;
    objeto = json_object_new_object();

    if (objeto == NULL) {
        return 0;
    }
    json_object_object_add(objeto, "type", json_object_new_string("RESPONSE"));
    json_object_object_add(objeto, "operation", json_object_new_string(operacion));
    json_object_object_add(objeto, "result", json_object_new_string(resultado));

    if (extra != NULL) {
        json_object_object_add(objeto, "extra", json_object_new_string(extra));
    }
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) {
        json_object_put(objeto);
        return 0;
    }
    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n';
    respuesta[longitud + 1] = '\0';
    json_object_put(objeto);
    return 1;
}

// Extrae y valida el nombre de la sala y la lista de usuarios de una invitación
int extraer_invitacion(const char *mensaje, char *nombre_sala, size_t sala_size, char ***usuarios, size_t *cantidad_usuarios) {
    struct json_object *objeto;
    struct json_object *tipo;
    struct json_object *sala;
    struct json_object *lista;
    const char *texto_sala;
    size_t cantidad;
    char **nombres;
    *usuarios = NULL;
    *cantidad_usuarios = 0;
    objeto = json_tokener_parse(mensaje);

    if (objeto == NULL || !json_object_is_type(objeto, json_type_object)) {
        if (objeto != NULL) {
            json_object_put(objeto);
        }
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "type", &tipo) || !json_object_is_type(tipo, json_type_string) || strcmp(json_object_get_string(tipo), "INVITE") != 0) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "roomname", &sala) || !json_object_is_type(sala, json_type_string)) {
        json_object_put(objeto);
        return 0;
    }
    texto_sala = json_object_get_string(sala);

    if (texto_sala == NULL || strlen(texto_sala) == 0 || strlen(texto_sala) >= sala_size) {
        json_object_put(objeto);
        return 0;
    }
    strcpy(nombre_sala, texto_sala);

    if (!json_object_object_get_ex(objeto, "usernames", &lista) || !json_object_is_type(lista, json_type_array)) {
        json_object_put(objeto);
        return 0;
    }
    cantidad = json_object_array_length(lista);

    if (cantidad == 0) {
        json_object_put(objeto);
        return 0;
    }
    nombres = malloc(sizeof(char *) * cantidad);

    if (nombres == NULL) {
        json_object_put(objeto);
        return 0;
    }
    // Recorro todos los elementos del arreglo para validar y copiar cada nombre de usuario a memoria propia
    for (size_t i = 0; i < cantidad; i++) {
        struct json_object *usuario;
        const char *nombre;
        size_t longitud;
        usuario = json_object_array_get_idx(lista, i);

        if (usuario == NULL || !json_object_is_type(usuario, json_type_string)) {
            // Libero los nombres que ya se habían copiado antes de encontrar el elemento inválido
            for (size_t j = 0; j < i; j++) {
                free(nombres[j]);
            }
            free(nombres);
            json_object_put(objeto);
            return 0;
        }
        nombre = json_object_get_string(usuario);
        longitud = strlen(nombre);

        if (nombre == NULL || longitud == 0 || longitud > 8) {
            // Libero las cadenas reservadas antes de detectar que el nombre actual no es válido
            for (size_t j = 0; j < i; j++) {
                free(nombres[j]);
            }
            free(nombres);
            json_object_put(objeto);
            return 0;
        }
        nombres[i] = malloc(longitud + 1);

        if (nombres[i] == NULL) {
            // Libero todas las cadenas reservadas hasta este momento porque ya no pudimos reservar memoria para la actual
            for (size_t j = 0; j < i; j++) {
                free(nombres[j]);
            }
            free(nombres);
            json_object_put(objeto);
            return 0;
        }
        strcpy(nombres[i], nombre);
    }
    *usuarios = nombres;
    *cantidad_usuarios = cantidad;
    json_object_put(objeto);
    return 1;
}

// Extrae y valida el nombre de una sala para una solicitud JOIN_ROOM
int extraer_union_sala(const char *mensaje, char *nombre_sala, size_t sala_size) {
    struct json_object *objeto;
    struct json_object *tipo;
    struct json_object *sala;
    const char *texto_sala;
    objeto = json_tokener_parse(mensaje);

    if (objeto == NULL) {
        return 0;
    }

    if (!json_object_is_type(objeto, json_type_object)) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "type", &tipo) || !json_object_is_type(tipo, json_type_string) || strcmp(json_object_get_string(tipo), "JOIN_ROOM") != 0) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "roomname", &sala) || !json_object_is_type(sala, json_type_string)) {
        json_object_put(objeto);
        return 0;
    }
    texto_sala = json_object_get_string(sala);

    if (texto_sala == NULL || strlen(texto_sala) == 0 || strlen(texto_sala) >= sala_size) {
        json_object_put(objeto);
        return 0;
    }
    strcpy(nombre_sala, texto_sala);
    json_object_put(objeto);
    return 1;
}

// Crea el mensaje que notifica que un usuario se unió a una sala
int crear_usuario_unido(const char *nombre_usuario, const char *nombre_sala, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    const char *texto_json;
    size_t longitud;
    objeto = json_object_new_object();

    if (objeto == NULL) {
        return 0;
    }
    json_object_object_add(objeto, "type", json_object_new_string("JOINED_ROOM"));
    json_object_object_add(objeto, "roomname", json_object_new_string(nombre_sala));
    json_object_object_add(objeto, "username", json_object_new_string(nombre_usuario));
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) {
        json_object_put(objeto);
        return 0;
    }
    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n';
    respuesta[longitud + 1] = '\0';
    json_object_put(objeto);
    return 1;
}

// Crea el mensaje JSON de invitación para un usuario
int crear_invitacion(const char *nombre_usuario, const char *nombre_sala, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    const char *texto_json;
    size_t longitud;
    objeto = json_object_new_object();

    if (objeto == NULL) {
        return 0;
    }
    json_object_object_add(objeto, "type", json_object_new_string("INVITATION"));
    json_object_object_add(objeto, "username", json_object_new_string(nombre_usuario));
    json_object_object_add(objeto, "roomname", json_object_new_string(nombre_sala));
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) {
        json_object_put(objeto);
        return 0;
    }
    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n';
    respuesta[longitud + 1] = '\0';
    json_object_put(objeto);
    return 1;
}

// Extrae y valida el nombre de una sala para una solicitud ROOM_USERS
int extraer_usuarios_sala(const char *mensaje, char *nombre_sala, size_t sala_size) {
    struct json_object *objeto;
    struct json_object *tipo;
    struct json_object *sala;
    objeto = json_tokener_parse(mensaje);

    if (objeto == NULL) {
        return 0;
    }

    if (!json_object_is_type(objeto, json_type_object)) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "type", &tipo) || !json_object_is_type(tipo, json_type_string) || strcmp(json_object_get_string(tipo), "ROOM_USERS") != 0) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "roomname", &sala) || !json_object_is_type(sala, json_type_string)) {
        json_object_put(objeto);
        return 0;
    }
    const char *nombre = json_object_get_string(sala);

    if (nombre == NULL || strlen(nombre) == 0 || strlen(nombre) >= sala_size) {
        json_object_put(objeto);
        return 0;
    }
    strcpy(nombre_sala, nombre);
    json_object_put(objeto);
    return 1;
}

// Crea la respuesta ROOM_USER_LIST con los miembros y sus estados
int crear_lista_usuarios_sala(const char *nombre_sala, const char *nombres[], const char *estados[], size_t cantidad, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    struct json_object *lista_usuarios;
    const char *texto_json;
    size_t longitud;
    objeto = json_object_new_object();

    if (objeto == NULL) {
        return 0;
    }
    lista_usuarios = json_object_new_object();

    if (lista_usuarios == NULL) {
        json_object_put(objeto);
        return 0;
    }
    json_object_object_add(objeto, "type", json_object_new_string("ROOM_USER_LIST"));
    json_object_object_add(objeto, "roomname", json_object_new_string(nombre_sala));
    // Agrego cada miembro de la sala junto con su estado
    for (size_t i = 0; i < cantidad; i++) {
        json_object_object_add(lista_usuarios, nombres[i], json_object_new_string(estados[i]));
    }
    json_object_object_add(objeto, "users", lista_usuarios);
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) {
        json_object_put(objeto);
        return 0;
    }
    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n';
    respuesta[longitud + 1] = '\0';
    json_object_put(objeto);
    return 1;
}

// Extrae y valida la sala y el texto de una solicitud ROOM_TEXT.
int extraer_texto_sala(const char *mensaje,
                       char *nombre_sala,
                       size_t sala_size,
                       char **texto) {
    struct json_object *objeto;
    struct json_object *tipo;
    struct json_object *sala;
    struct json_object *texto_json;
    const char *nombre;
    const char *contenido;

    *texto = NULL;

    objeto = json_tokener_parse(mensaje);

    if (objeto == NULL ||
        !json_object_is_type(objeto, json_type_object)) {
        if (objeto != NULL) {
            json_object_put(objeto);
        }
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "type", &tipo) ||
        !json_object_is_type(tipo, json_type_string) ||
        strcmp(json_object_get_string(tipo), "ROOM_TEXT") != 0) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "roomname", &sala) ||
        !json_object_is_type(sala, json_type_string)) {
        json_object_put(objeto);
        return 0;
    }

    nombre = json_object_get_string(sala);

    if (nombre == NULL ||
        strlen(nombre) == 0 ||
        strlen(nombre) >= sala_size) {
        json_object_put(objeto);
        return 0;
    }

    if (!json_object_object_get_ex(objeto, "text", &texto_json) ||
        !json_object_is_type(texto_json, json_type_string)) {
        json_object_put(objeto);
        return 0;
    }

    contenido = json_object_get_string(texto_json);

    if (contenido == NULL) {
        json_object_put(objeto);
        return 0;
    }

    strncpy(nombre_sala, nombre, sala_size - 1);
    nombre_sala[sala_size - 1] = '\0';

    *texto = malloc(strlen(contenido) + 1);

    if (*texto == NULL) {
        json_object_put(objeto);
        return 0;
    }

    strcpy(*texto, contenido);

    json_object_put(objeto);
    return 1;
}

// Crea el mensaje ROOM_TEXT_FROM que se envía a los miembros de una sala
int crear_texto_sala_desde(const char *nombre_usuario, const char *nombre_sala, const char *texto, char *respuesta, size_t respuesta_size) {
    struct json_object *objeto;
    const char *texto_json;
    size_t longitud;
    objeto = json_object_new_object();

    if (objeto == NULL) {
        return 0;
    }
    json_object_object_add(objeto, "type", json_object_new_string("ROOM_TEXT_FROM"));
    json_object_object_add(objeto, "roomname", json_object_new_string(nombre_sala));
    json_object_object_add(objeto, "username", json_object_new_string(nombre_usuario));
    json_object_object_add(objeto, "text", json_object_new_string(texto));
    texto_json = json_object_to_json_string(objeto);
    longitud = strlen(texto_json);

    if (longitud + 2 > respuesta_size) {
        json_object_put(objeto);
        return 0;
    }
    memcpy(respuesta, texto_json, longitud);
    respuesta[longitud] = '\n';
    respuesta[longitud + 1] = '\0';
    json_object_put(objeto);
    return 1;
}