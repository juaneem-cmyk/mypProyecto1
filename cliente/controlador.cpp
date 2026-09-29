#include <json-c/json.h>
#include <cstring>
#include <cstdio>
#include <Controlador.h>

// Constructor de mensaje de identificación en formato JSON para enviar al servidor
std::string Controlador::crear_identificacion(const std::string& usuario) {
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return ""; // Error al crear el objeto JSON
    }
    json_object_object_add(objeto, "type", json_object_new_string("IDENTIFY"));
    json_object_object_add(objeto, "username", json_object_new_string(usuario.c_str()));
    
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    
    json_object_put(objeto); // Liberar memoria del objeto JSON
    return mensaje;
}

// Crea el mensaje JSON para solicitar la lista de usuarios al servidor
std::string Controlador::crear_solicitud_usuarios() {
    {
    std::lock_guard<std::mutex> bloqueo(mutex_usuarios);
    lista_usuarios_recibida = false;
    }
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return "";
    }

    json_object_object_add( objeto, "type", json_object_new_string("USERS"));
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    json_object_put(objeto);
    return mensaje;
}

// Crea el mensaje JSON para solicitar la desconexión del cliente
std::string Controlador::crear_desconexion() {
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return "";
    }
    json_object_object_add(objeto, "type", json_object_new_string("DISCONNECT"));
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    json_object_put(objeto);
    return mensaje;
}

// Crea el mensaje JSON para enviar un texto privado a otro usuario
std::string Controlador::crear_texto(const std::string& destinatario, const std::string& texto) {
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return "";
    }
    // Agrego el tipo de operación
    json_object_object_add(objeto, "type", json_object_new_string("TEXT"));
    // Agrego el nombre del usuario destinatario
    json_object_object_add(objeto, "username", json_object_new_string(destinatario.c_str()));
    // Agrego el contenido del mensaje
    json_object_object_add(objeto, "text", json_object_new_string(texto.c_str()));
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    json_object_put(objeto);
    return mensaje;
}

// Crea el mensaje para enviar un texto público
std::string Controlador::crear_texto_publico(const std::string& texto) {
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return "";
    }
    json_object_object_add(objeto, "type", json_object_new_string("PUBLIC_TEXT"));
    json_object_object_add(objeto, "text", json_object_new_string(texto.c_str()));
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    json_object_put(objeto);
    return mensaje;
}

// Crea el mensaje JSON para cambiar el estado del usuario
std::string Controlador::crear_status(const std::string& status) {
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return "";
    }
    json_object_object_add(objeto, "type", json_object_new_string("STATUS"));
    json_object_object_add(objeto, "status", json_object_new_string(status.c_str()));
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    json_object_put(objeto);
    return mensaje;
}

// Crea el mensaje JSON para solicitar la creación de una nueva sala
std::string Controlador::crear_nueva_sala(const std::string& nombre_sala) {
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return "";
    }

    json_object_object_add(objeto, "type", json_object_new_string("NEW_ROOM"));
    json_object_object_add(objeto, "roomname", json_object_new_string(nombre_sala.c_str()));
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    json_object_put(objeto);
    return mensaje;
}

// Crea el mensaje para invitar usuarios a una sala
std::string Controlador::crear_invitacion(const std::string& nombre_sala, const std::vector<std::string>& usuarios) {
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return "";
    }
    struct json_object *lista_usuarios = json_object_new_array();

    if (lista_usuarios == nullptr) {
        json_object_put(objeto);
        return "";
    }
    json_object_object_add(objeto, "type", json_object_new_string("INVITE"));
    json_object_object_add(objeto, "roomname", json_object_new_string(nombre_sala.c_str()));
    // Agrego cada nombre al arreglo de usuarios
    for (const std::string& usuario : usuarios) {
        json_object_array_add(lista_usuarios, json_object_new_string(usuario.c_str()));
    }
    json_object_object_add(objeto, "usernames", lista_usuarios);
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    json_object_put(objeto);
    return mensaje;
}

// Crea el mensaje para solicitar los usuarios de una sala
std::string Controlador::crear_usuarios_sala(const std::string& nombre_sala) {
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return "";
    }
    json_object_object_add(objeto, "type", json_object_new_string("ROOM_USERS"));
    json_object_object_add(objeto, "roomname", json_object_new_string(nombre_sala.c_str()));
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    json_object_put(objeto);
    return mensaje;
}

// Crea el mensaje JSON para enviar texto a una sala
std::string Controlador::crear_texto_sala(const std::string& nombre_sala, const std::string& texto) {
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return "";
    }
    json_object_object_add(objeto, "type", json_object_new_string("ROOM_TEXT"));
    json_object_object_add(objeto, "roomname", json_object_new_string(nombre_sala.c_str()));
    json_object_object_add(objeto, "text", json_object_new_string(texto.c_str()));
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    json_object_put(objeto);
    return mensaje;
}

// Crea el mensaje para solicitar unirse a una sala
std::string Controlador::crear_unirse_sala(const std::string& nombre_sala) {
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return "";
    }

    json_object_object_add(objeto, "type", json_object_new_string("JOIN_ROOM"));
    json_object_object_add(objeto, "roomname", json_object_new_string(nombre_sala.c_str()));
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    json_object_put(objeto);
    return mensaje;
}

// Crea el mensaje JSON para abandonar una sala
std::string Controlador::crear_salir_sala(const std::string& nombre_sala) {
    struct json_object *objeto = json_object_new_object();

    if (objeto == nullptr) {
        return "";
    }
    json_object_object_add(objeto, "type", json_object_new_string("LEAVE_ROOM"));
    json_object_object_add(objeto, "roomname", json_object_new_string(nombre_sala.c_str()));
    const char *mensaje_json = json_object_to_json_string(objeto);
    std::string mensaje(mensaje_json);
    json_object_put(objeto);
    return mensaje;
}

// Espera hasta que el hilo de lectura reciba USER_LIST
void Controlador::esperar_lista_usuarios() {
    std::unique_lock<std::mutex> bloqueo(mutex_usuarios);
    condicion_usuarios.wait(bloqueo, [this] {return lista_usuarios_recibida;});
}

// Devuelve una copia de la lista de usuarios para que el hilo principal pueda consultarla
std::vector<std::pair<std::string, std::string>> Controlador::obtener_usuarios() {
    std::lock_guard<std::mutex> bloqueo(mutex_usuarios);
    return usuarios;
}

// Convierte las respuestas del protocolo en mensajes para el usuario
static void mostrar_respuesta(const char *operacion, const char *resultado, const char *extra) {
    if (strcmp(operacion, "IDENTIFY") == 0) {
        if (strcmp(resultado, "SUCCESS") == 0) {
            printf("¡Bienvenido, %s! Te has conectado al chat.\n", extra);
        } else if (strcmp(resultado, "USER_ALREADY_EXISTS") == 0) {
            printf("El usuario \"%s\" ya está conectado.\n", extra);
        } else {
            printf("No fue posible identificar al usuario.\n");
        }
        return;
    }

    if (strcmp(operacion, "STATUS") == 0) {
        if (strcmp(resultado, "SUCCESS") == 0) {
            printf("Tu estado se actualizó correctamente.\n");
        } else {
            printf("No fue posible actualizar tu estado.\n");
        }
        return;
    }

    if (strcmp(operacion, "TEXT") == 0) {
        if (strcmp(resultado, "NO_SUCH_USER") == 0) {
            printf("El usuario \"%s\" no existe.\n", extra);
        } else {
            printf("No fue posible enviar el mensaje.\n");
        }
        return;
    }

    if (strcmp(operacion, "NEW_ROOM") == 0) {
        if (strcmp(resultado, "SUCCESS") == 0) {
            printf("La sala \"%s\" fue creada correctamente.\n", extra);
        } else if (strcmp(resultado, "ROOM_ALREADY_EXISTS") == 0) {
            printf("La sala \"%s\" ya existe.\n", extra);
        } else {
            printf("No fue posible crear la sala.\n");
        }
        return;
    }

    if (strcmp(operacion, "INVITE") == 0) {
        if (strcmp(resultado, "NO_SUCH_ROOM") == 0) {
            printf("La sala \"%s\" no existe.\n", extra);
        } else if (strcmp(resultado, "NO_SUCH_USER") == 0) {
            printf("El usuario \"%s\" no existe.\n", extra);
        } else if (strcmp(resultado, "NOT_MEMBER") == 0) {
            printf("No perteneces a esa sala y no puedes invitar usuarios.\n");
        } else {
            printf("No fue posible enviar la invitación.\n");
        }
        return;
    }

    if (strcmp(operacion, "JOIN_ROOM") == 0) {
        if (strcmp(resultado, "SUCCESS") == 0) {
            printf("Te has unido a la sala \"%s\".\n", extra);
        } else if (strcmp(resultado, "NO_SUCH_ROOM") == 0) {
            printf("La sala \"%s\" no existe.\n", extra);
        } else if (strcmp(resultado, "NOT_INVITED") == 0) {
            printf("No tienes una invitación para la sala \"%s\".\n", extra);
        } else {
            printf("No fue posible unirte a la sala.\n");
        }
        return;
    }

    if (strcmp(operacion, "ROOM_USERS") == 0) {
        if (strcmp(resultado, "NO_SUCH_ROOM") == 0) {
            printf("La sala \"%s\" no existe.\n", extra);
        } else if (strcmp(resultado, "NOT_JOINED") == 0) {
            printf("No estás dentro de la sala \"%s\".\n", extra);
        } else {
            printf("No fue posible consultar los usuarios de la sala.\n");
        }
        return;
    }

    if (strcmp(operacion, "ROOM_TEXT") == 0) {
        if (strcmp(resultado, "NO_SUCH_ROOM") == 0) {
            printf("La sala \"%s\" no existe.\n", extra);
        } else if (strcmp(resultado, "NOT_JOINED") == 0) {
            printf("No estás dentro de la sala \"%s\".\n", extra);
        } else {
            printf("No fue posible enviar el mensaje a la sala.\n");
        }
        return;
    }

    if (strcmp(operacion, "LEAVE_ROOM") == 0) {
        if (strcmp(resultado, "NO_SUCH_ROOM") == 0) {
            printf("La sala \"%s\" no existe.\n", extra);
        } else if (strcmp(resultado, "NOT_JOINED") == 0) {
            printf("No estás dentro de la sala \"%s\".\n", extra);
        } else {
            printf("No fue posible salir de la sala.\n");
        }
        return;
    }

    if (strcmp(operacion, "INVALID") == 0) {
        if (strcmp(resultado, "NOT_IDENTIFIED") == 0) {
            printf("Debes identificarte antes de realizar esa acción.\n");
        } else {
            printf("El mensaje enviado no es válido.\n");
        }
        return;
    }
    printf("El servidor respondió con un resultado no reconocido.\n");
}

// Procesa los mensajes recibidos del servidor
void Controlador::procesar_mensaje(const std::string& mensaje) {
    json_object *objeto = json_tokener_parse(mensaje.c_str());

    if (objeto == nullptr) {
        fprintf(stderr, "Error al procesar el mensaje.\n");
        return;
    }
    json_object *tipo;

    if (!json_object_object_get_ex(objeto, "type", &tipo) || !json_object_is_type(tipo, json_type_string)) {
        fprintf(stderr, "Error: mensaje sin un tipo válido.\n");
        json_object_put(objeto);
        return;
    }
    const char *tipo_texto = json_object_get_string(tipo);

    // Procesa las respuestas del servidor
    if (strcmp(tipo_texto, "RESPONSE") == 0) {
        json_object *operacion;
        json_object *resultado;
        json_object *extra;
        const char *texto_operacion = "";
        const char *texto_resultado = "";
        const char *texto_extra = "";

        if (json_object_object_get_ex(objeto, "operation", &operacion) && json_object_is_type(operacion, json_type_string)) {
            texto_operacion = json_object_get_string(operacion);
        }

        if (json_object_object_get_ex(objeto, "result", &resultado) && json_object_is_type(resultado, json_type_string)) {
            texto_resultado = json_object_get_string(resultado);
        }

        if (json_object_object_get_ex(objeto, "extra", &extra) && json_object_is_type(extra, json_type_string)) {
            texto_extra = json_object_get_string(extra);
        }
        mostrar_respuesta(texto_operacion, texto_resultado, texto_extra);
        json_object_put(objeto);
        return;
    }

    // Procesa una invitación recibida para una sala
    if (strcmp(tipo_texto, "INVITATION") == 0) {
        json_object *usuario;
        json_object *sala;

        if (json_object_object_get_ex(objeto, "username", &usuario) && json_object_object_get_ex(objeto, "roomname", &sala) &&
            json_object_is_type(usuario, json_type_string) && json_object_is_type(sala, json_type_string)) {
            printf("Has recibido una invitación de %s para la sala \"%s\".\n", json_object_get_string(usuario), json_object_get_string(sala));
        }
        json_object_put(objeto);
        return;
    }

    // Informa que un usuario se unió a una sala
    if (strcmp(tipo_texto, "JOINED_ROOM") == 0) {
        json_object *usuario;
        json_object *sala;

        if (json_object_object_get_ex(objeto, "username", &usuario) && json_object_object_get_ex(objeto, "roomname", &sala) &&
            json_object_is_type(usuario, json_type_string) && json_object_is_type(sala, json_type_string)) {
            printf("%s se unió a la sala \"%s\".\n", json_object_get_string(usuario), json_object_get_string(sala));
        }
        json_object_put(objeto);
        return;
    }

    // Procesa la lista de usuarios recibida del servidor
    if (strcmp(tipo_texto, "USER_LIST") == 0) {
        json_object *lista_usuarios;

        if (!json_object_object_get_ex(objeto, "users", &lista_usuarios) ||
            !json_object_is_type(lista_usuarios, json_type_object)) {
            fprintf(stderr, "Error: USER_LIST no contiene un objeto 'users'.\n");
            json_object_put(objeto);
            return;
        }
        std::lock_guard<std::mutex> bloqueo(mutex_usuarios);
        usuarios.clear();
        json_object_object_foreach(lista_usuarios, nombre, estado) {
            usuarios.push_back({nombre, json_object_get_string(estado)});
        }
        lista_usuarios_recibida = true;
        condicion_usuarios.notify_all();
        json_object_put(objeto);
        return;
    }

    // Procesa un mensaje privado recibido
    if (strcmp(tipo_texto, "TEXT_FROM") == 0) {
        json_object *usuario;
        json_object *texto;

        if (json_object_object_get_ex(objeto, "username", &usuario) && json_object_object_get_ex(objeto, "text", &texto) &&
            json_object_is_type(usuario, json_type_string) && json_object_is_type(texto, json_type_string)) {
            printf("%s: %s\n", json_object_get_string(usuario), json_object_get_string(texto));
        }
        json_object_put(objeto);
        return;
    }

    // Procesa la lista de usuarios de una sala
    if (strcmp(tipo_texto, "ROOM_USER_LIST") == 0) {
        json_object *sala;
        json_object *lista_usuarios;

        if (json_object_object_get_ex(objeto, "roomname", &sala) && json_object_object_get_ex(objeto, "users", &lista_usuarios) &&
            json_object_is_type(sala, json_type_string) && json_object_is_type(lista_usuarios, json_type_object)) {
            printf("Usuarios de la sala \"%s\":\n", json_object_get_string(sala));
            json_object_object_foreach(lista_usuarios, nombre, estado) {
                printf("  - %s [%s]\n", nombre, json_object_get_string(estado));
            }
        }
        json_object_put(objeto);
        return;
    }

    // Procesa un mensaje recibido desde una sala
    if (strcmp(tipo_texto, "ROOM_TEXT_FROM") == 0) {
        json_object *usuario;
        json_object *sala;
        json_object *texto;

        if (json_object_object_get_ex(objeto, "username", &usuario) && json_object_object_get_ex(objeto, "roomname", &sala) &&
            json_object_object_get_ex(objeto, "text", &texto) && json_object_is_type(usuario, json_type_string) &&
            json_object_is_type(sala, json_type_string) && json_object_is_type(texto, json_type_string)) {
            printf("[%s] %s: %s\n", json_object_get_string(sala), json_object_get_string(usuario), json_object_get_string(texto));
        }
        json_object_put(objeto);
        return;
    }

    // Informa que un usuario salió de una sala
    if (strcmp(tipo_texto, "LEFT_ROOM") == 0) {
        json_object *usuario;
        json_object *sala;

        if (json_object_object_get_ex(objeto, "username", &usuario) && json_object_object_get_ex(objeto, "roomname", &sala) &&
            json_object_is_type(usuario, json_type_string) && json_object_is_type(sala, json_type_string)) {
            printf("%s salió de la sala \"%s\".\n", json_object_get_string(usuario), json_object_get_string(sala));
        }
        json_object_put(objeto);
        return;
    }

    // Informa que un usuario se desconectó
    if (strcmp(tipo_texto, "DISCONNECTED") == 0) {
        json_object *usuario;

        if (json_object_object_get_ex(objeto, "username", &usuario) && json_object_is_type(usuario, json_type_string)) {
            printf("%s se desconectó del chat.\n", json_object_get_string(usuario));
        }
        json_object_put(objeto);
        return;
    }

    // Procesa un mensaje público recibido
    if (strcmp(tipo_texto, "PUBLIC_TEXT_FROM") == 0) {
        json_object *usuario;
        json_object *texto;

        if (json_object_object_get_ex(objeto, "username", &usuario) && json_object_object_get_ex(objeto, "text", &texto) &&
            json_object_is_type(usuario, json_type_string) && json_object_is_type(texto, json_type_string)) {
            printf("[Público] %s: %s\n", json_object_get_string(usuario), json_object_get_string(texto));
        }
        json_object_put(objeto);
        return;
    }

    // Informa que un nuevo usuario se identificó
    if (strcmp(tipo_texto, "NEW_USER") == 0) {
        json_object *usuario;

        if (json_object_object_get_ex(objeto, "username", &usuario) && json_object_is_type(usuario, json_type_string)) {
            printf("%s se ha conectado al chat.\n", json_object_get_string(usuario));
        }
        json_object_put(objeto);
        return;
    }

    // Informa que un usuario cambió su estado
    if (strcmp(tipo_texto, "NEW_STATUS") == 0) {
        json_object *usuario;
        json_object *estado;

        if (json_object_object_get_ex(objeto, "username", &usuario) && json_object_object_get_ex(objeto, "status", &estado) &&
            json_object_is_type(usuario, json_type_string) && json_object_is_type(estado, json_type_string)) {
            const char *estado_texto = json_object_get_string(estado);
            const char *estado_amigable = estado_texto;

            if (strcmp(estado_texto, "ACTIVE") == 0) {
                estado_amigable = "activo";
            } else if (strcmp(estado_texto, "AWAY") == 0) {
                estado_amigable = "ausente";
            } else if (strcmp(estado_texto, "BUSY") == 0) {
                estado_amigable = "ocupado";
            }
            printf("%s ahora está %s.\n", json_object_get_string(usuario), estado_amigable);
        }
        json_object_put(objeto);
        return;
    }
    json_object_put(objeto);
}