#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "TCPCliente.h"

/* Encontré 2 repositorios de referencia y un libro que habla de un proyecto multiusuario de chat 
https://beej.us/guide/bgnet0/
https://github.com/AmineToualbi/TCPChat.git
https://github.com/BaseMax/TCP-Chat-CPP.git
*/
// Constructor de la clase TCPCliente
TCPCliente::TCPCliente(const std::string ip, int puerto) : socket_cliente(-1), ip(ip), puerto(puerto) {
}
TCPCliente::~TCPCliente() {
    if (socket_cliente != -1) {
        close(socket_cliente);
    }
}
// Método para conectar al servidor
bool TCPCliente::conectar() {
    socket_cliente = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_cliente == -1) {
        perror("Error al crear el socket");
        return false;
    }

    sockaddr_in direccion_servidor;
    direccion_servidor.sin_family = AF_INET;
    direccion_servidor.sin_port = htons(puerto);

    if (connect(socket_cliente, reinterpret_cast<sockaddr*>(&direccion_servidor), sizeof(direccion_servidor)) == -1) {
        perror("Error al conectar con el servidor");
        close(socket_cliente);
        socket_cliente = -1;
        return false;
    }
    std::cout << "Conexión establecida con el servidor " << ip << ":" << puerto << std::endl;
    return true;
}