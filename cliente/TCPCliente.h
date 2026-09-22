#ifndef TCPCliente_H
#define TCPCliente_H

#include <string>
#include <atomic>
#include <thread>
#include <functional>
// Clase TCPCliente para manejar la conexión TCP con el servidor
class TCPCliente {
    private:
        int socket_cliente;
        std::string ip;
        int puerto;

        std::atomic<bool> activo;
        std::thread hilo_lectura;
        std::string buffer_entrada;
        std::function<void(const std::string&)> manejador_mensajes;
        
        static constexpr size_t max_mensaje = 1024 * 1024;
        void leer_mensajes();

    public:
        TCPCliente(const std::string& ip, int puerto);
        ~TCPCliente();
        bool conectar();
        bool enviar_mensaje(const std::string& mensaje);
        bool esta_activo() const;
        void establecer_manejador(std::function<void(const std::string&)> manejador);
};
#endif