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