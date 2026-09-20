#include <iostream>
#include <cstdio>
#include "TCPCliente.h"
#include "Controlador.h"

/*Esto lo iba a colocar en el reporte pero supongo que aquí también es importante,
  estoy leyendo un libro que se llama "Como programar en C/C++ y Java" de la editorial Prentice Hall 
  tiene unas hormigas en la portada :D*/

int main() {
  Controlador controlador;
  TCPCliente cliente("127.0.0.1", 1234);

  // Establecer el manejador de mensajes recibidos
  cliente.establecer_manejador([&controlador](const std::string& mensaje) {
    controlador.procesar_mensaje(mensaje);
  });

  // Intentar conectar al servidor
  if (!cliente.conectar()) {
    return 1;
  }

  // Enviar un mensaje de identificación al servidor
  printf("Cliente conectado al servidor.\n");
  std::string usuario;
  printf("Ingrese su nombre de usuario: ");
  std::getline(std::cin, usuario);
  std::string mensaje_identificacion = controlador.crear_identificacion(usuario);
  cliente.enviar_mensaje(mensaje_identificacion);

  // Bucle para enviar mensajes al servidor
  while (std::getline(std::cin, mensaje_identificacion)) {
    if (mensaje_identificacion == "salir") {
      break;
    }
    cliente.enviar_mensaje(mensaje_identificacion);
  }

  return 0;
}