#ifndef CONTROLADOR_H
#define CONTROLADOR_H

#include <string>
#include <vector>
#include <utility>
#include <mutex>
#include <condition_variable>

class Controlador {
    private:
    std::vector<std::pair<std::string, std::string>> usuarios;
    std::mutex mutex_usuarios;
    std::condition_variable condicion_usuarios;
    bool lista_usuarios_recibida = false;
    
    public:
    std::string crear_identificacion(const std::string& usuario);
    std::string crear_solicitud_usuarios();
    std::string crear_texto(const std::string& destinatario, const std::string& texto);
    std::string crear_status(const std::string& status);
    std::string crear_nueva_sala(const std::string& nombre_sala);
    std::string crear_invitacion(const std::string& nombre_sala, const std::vector<std::string>& usuarios);
    std::string crear_unirse_sala(const std::string& nombre_sala);
    std::string crear_usuarios_sala(const std::string& nombre_sala);
    std::string crear_texto_sala(const std::string& nombre_sala, const std::string& texto);
    std::string crear_salir_sala(const std::string& nombre_sala);
    std::string crear_desconexion();
    std::vector<std::pair<std::string, std::string>> obtener_usuarios();
    void procesar_mensaje(const std::string& mensaje);
    void esperar_lista_usuarios();
};

#endif