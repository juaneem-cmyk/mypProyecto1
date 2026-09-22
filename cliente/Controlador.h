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
    std::string crear_desconexion();
    std::vector<std::pair<std::string, std::string>> obtener_usuarios();
    void procesar_mensaje(const std::string& mensaje);
    void esperar_lista_usuarios();
};

#endif