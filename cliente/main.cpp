#include <iostream>
#include <cstdio>
#include "TCPCliente.h"

/*Esto lo iba a colocar en el reporte pero supongo que aquí también es importante,
  estoy leyendo un libro que se llama "Como programar en C/C++ y Java" de la editorial Prentice Hall 
  tiene unas hormigas en la portada :D*/

int main() {
  TCPCliente cliente("127.0.0.1", 1234);
  if (!cliente.conectar()) {
    return 1;
  }

  // Enviar un mensaje de identificación al servidor
  printf("Cliente conectado al servidor.\n");
  std::string usuario;
  printf("Ingrese su nombre de usuario: ");
  std::getline(std::cin, usuario);
  std::string mensaje = "{\"type\":\"IDENTIFY\",\"username\":\"" + usuario + "\"}";  
  cliente.enviar_mensaje(mensaje);

  // Bucle para enviar mensajes al servidor
  while (std::getline(std::cin, mensaje)) {
    if (mensaje == "salir") {
      break;
    }
    cliente.enviar_mensaje(mensaje);
  }

  return 0;
}