#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include "TCPCliente.h"

/* Encontré 2 repositorios de referencia y un libro que habla de un proyecto multiusuario de chat 
https://beej.us/guide/bgnet0/
https://github.com/AmineToualbi/TCPChat.git
https://github.com/BaseMax/TCP-Chat-CPP.git
*/
// Constructor de la clase TCPCliente
TCPCliente::TCPCliente(const std::string& ip, int puerto) : socket_cliente(-1), ip(ip), puerto(puerto), activo(false) {
}
// Destructor de la clase TCPCliente
TCPCliente::~TCPCliente() {
    activo = false;
    if (socket_cliente != -1) {
        shutdown(socket_cliente, SHUT_RDWR);
        close(socket_cliente);
    }
    if (hilo_lectura.joinable()) {
        hilo_lectura.join();
    }
}

// Establece el manejador de mensajes recibidos
void TCPCliente::establecer_manejador(std::function<void(const std::string&)> manejador) {
    manejador_mensajes = manejador;
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
    
    // Convertir la dirección IP de cadena a binario
    if (inet_pton(AF_INET, ip.c_str(), &direccion_servidor.sin_addr) <= 0) {
        perror("Error al convertir la dirección IP");
        close(socket_cliente);
        socket_cliente = -1;
        return false;
    }

    // Intentar conectar al servidor
    if (connect(socket_cliente, reinterpret_cast<sockaddr*>(&direccion_servidor), sizeof(direccion_servidor)) == -1) {
        perror("Error al conectar con el servidor");
        close(socket_cliente);
        socket_cliente = -1;
        return false;
    } 
    printf("Conexión establecida con el servidor %s:%d\n", ip.c_str(), puerto);    
    activo = true;
    hilo_lectura = std::thread(&TCPCliente::leer_mensajes, this);
    return true;
}

// Método para enviar un mensaje al servidor
bool TCPCliente::enviar_mensaje(const std::string& mensaje) {
    if (!activo || socket_cliente == -1) {
        fprintf(stderr, "No hay conexión establecida con el servidor.\n");        
        return false;
    }

    std::string mensaje_final = mensaje + "\n"; // Agregar un salto de línea al final del mensaje
    ssize_t bytes_enviados = send(socket_cliente, mensaje_final.c_str(), mensaje_final.size(), 0);
    if (bytes_enviados < 0) {
        perror("Error al enviar el mensaje");
        return false;
    }
    return true;
}

// Indica si el cliente sigue conectado al servidor.
bool TCPCliente::esta_activo() const {
    return activo.load();
}

// Hilo que se encarga de leer los mensajes del servidor
void TCPCliente::leer_mensajes() {
    char buffer[4096];
    while (activo) {
        ssize_t bytes_recibidos = recv(socket_cliente, buffer, sizeof(buffer), 0);

        if (bytes_recibidos > 0){
            buffer_entrada.append(buffer, bytes_recibidos);
            while (true) {
                size_t posicion = buffer_entrada.find('\n');

                if (posicion == std::string::npos) {
                    if (buffer_entrada.size() > max_mensaje){
                        fprintf(stderr, "Mensaje recibido demasiado grande.\n");
                        activo = false;
                    }
                    break;
                }
                if (posicion > max_mensaje){
                    fprintf(stderr, "Mensaje recibido demasiado grande.\n");
                    activo = false;
                    break;
                }

                std::string mensaje = buffer_entrada.substr(0,posicion);
                buffer_entrada.erase(0,posicion + 1);

                if (mensaje.empty()){
                    continue;
                }
                if (manejador_mensajes){
                    manejador_mensajes(mensaje);
                }
            }
        } else if (bytes_recibidos == 0) {
            printf("El servidor ha cerrado la conexión.\n");
            activo = false;
        } else {
            perror("Error al recibir los datos");
            activo = false;
        }
    }
}