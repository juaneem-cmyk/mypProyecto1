#include <json-c/json.h>
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

// Procesa un mensaje JSON recibido del servidor
void Controlador::procesar_mensaje(const std::string& mensaje) {
    json_object *objeto = json_tokener_parse(mensaje.c_str());

    if (objeto == nullptr) {
        fprintf(stderr, "Error al procesar el mensaje JSON: %s\n");
        return;
    }
    json_object *tipo;
    if (!json_object_object_get_ex(objeto, "type", &tipo)) {
        fprintf(stderr, "Error: No se encontró el campo 'type' en el mensaje JSON\n");
        json_object_put(objeto);
        return;
    }

    printf("Mensaje recibido:\n%s\n", json_object_get_string(tipo));
    json_object_put(objeto); // Liberar memoria del objeto JSON
}