#ifndef TCPCliente_H
#define TCPCliente_H
#include <string>
// Clase TCPCliente para manejar la conexión TCP con el servidor
class TCPCliente {
    private:
        int socket_cliente;
        std::string ip;
        int puerto;

    public:
        TCPCliente(const std::string ip, int puerto);
        ~TCPCliente();
        bool conectar();
};
#endif