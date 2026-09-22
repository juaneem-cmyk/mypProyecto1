#ifndef CONTROLADOR_H
#define CONTROLADOR_H

#include <string>
#include <vector>
#include <utility>
#include <mutex>

class Controlador {
    private:
    std::vector<std::pair<std::string, std::string>> usuarios;
    std::mutex mutex_usuarios;
    
    public:
    std::string crear_identificacion(const std::string& usuario);
    std::string crear_solicitud_usuarios();
    std::string crear_texto(const std::string& destinatario, const std::string& texto);
    void procesar_mensaje(const std::string& mensaje);
};

#endif