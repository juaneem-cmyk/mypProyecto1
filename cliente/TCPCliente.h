#ifndef TCPCliente_H
#define TCPCliente_H

#include <string>
#include <atomic>
#include <thread>
// Clase TCPCliente para manejar la conexión TCP con el servidor
class TCPCliente {
    private:
        int socket_cliente;
        std::string ip;
        int puerto;

        std::atomic<bool> activo;
        std::thread hilo_lectura;
        void leer_mensajes();

    public:
        TCPCliente(const std::string& ip, int puerto);
        ~TCPCliente();
        bool conectar();
        bool enviar_mensaje(const std::string& mensaje);
};
#endif