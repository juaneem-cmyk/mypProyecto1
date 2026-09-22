#ifndef CONTROLADOR_H
#define CONTROLADOR_H

#include <string>

class Controlador {
public:
    std::string crear_identificacion(const std::string& usuario);
    std::string crear_texto(const std::string& destinatario, const std::string& texto);
    void procesar_mensaje(const std::string& mensaje);
};

#endif