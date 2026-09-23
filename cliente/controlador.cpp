#include <json-c/json.h>
#include <cstring>
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

// Procesa un mensaje JSON recibido del servidor
void Controlador::procesar_mensaje(const std::string& mensaje) {
    json_object *objeto = json_tokener_parse(mensaje.c_str());

    if (objeto == nullptr) {
        fprintf(stderr, "Error al procesar el mensaje JSON \n");
        return;
    }
    json_object *tipo;
    if (!json_object_object_get_ex(objeto, "type", &tipo)) {
        fprintf(stderr, "Error: No se encontró el campo 'type' en el mensaje JSON\n");
        json_object_put(objeto);
        return;
    }

    printf("Tipo de mensaje recibido: %s\n", json_object_get_string(tipo));
    
    // Procesar el mensaje según su tipo
    if (strcmp(json_object_get_string(tipo), "RESPONSE") == 0) {
        json_object *operacion;
        json_object *resultado;
        json_object *extra;
        
        if (json_object_object_get_ex(objeto, "operation", &operacion) &&
        json_object_object_get_ex(objeto, "result", &resultado) &&
        json_object_object_get_ex(objeto, "extra", &extra)) {
            
            printf("Operación: %s\n", json_object_get_string(operacion));
            printf("Resultado: %s\n", json_object_get_string(resultado));
            printf("Extra: %s\n", json_object_get_string(extra));
        }
    }
    
    // Procesar la lista de usuarios recibida del servidor
    if (strcmp(json_object_get_string(tipo), "USER_LIST") == 0) {
        json_object *lista_usuarios;

        if (!json_object_object_get_ex(objeto, "users", &lista_usuarios) || !json_object_is_type(lista_usuarios, json_type_object)) {
            fprintf(stderr, "Error: USER_LIST no contiene un objeto 'users'.\n");
            json_object_put(objeto);
            return;
        }
        std::lock_guard<std::mutex> bloqueo(mutex_usuarios);
        usuarios.clear();
        json_object_object_foreach(lista_usuarios, nombre, estado) {
            usuarios.push_back({nombre, json_object_get_string(estado)});
        }
        // Indico que ya recibí la lista y despierto al hilo que la estaba esperando
        lista_usuarios_recibida = true;
        condicion_usuarios.notify_all();
    }

    // Procesar un mensaje privado recibido
    if (strcmp(json_object_get_string(tipo), "TEXT_FROM") == 0) {
        json_object *usuario;
        json_object *texto;

        if (json_object_object_get_ex(objeto, "username", &usuario) && json_object_object_get_ex(objeto, "text", &texto) && 
        json_object_is_type(usuario, json_type_string) && json_object_is_type(texto, json_type_string)) {
            printf("\nMensaje privado de %s: %s\n", json_object_get_string(usuario), json_object_get_string(texto));
        }
    }
    json_object_put(objeto); // Liberar memoria del objeto JSON
}