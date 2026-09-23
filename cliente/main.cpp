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

  // Espera los comandos que escriba el usuario
  std::string comando;

  while (cliente.esta_activo() && std::getline(std::cin, comando)) {
    // Permite cerrar el cliente desde la consola.
    if (comando == "DISCONNECT") {
        std::string mensaje_desconexion = controlador.crear_desconexion();
        cliente.enviar_mensaje(mensaje_desconexion);
        break;
    }

    // Solicita la lista de usuarios al servidor.
    if (comando == "USERS") {
        std::string mensaje = controlador.crear_solicitud_usuarios();
        cliente.enviar_mensaje(mensaje);
    }

    // Inicia el proceso para enviar un mensaje privado.
    else if (comando == "TEXT") {
      std::string mensaje = controlador.crear_solicitud_usuarios();
      cliente.enviar_mensaje(mensaje);
      controlador.esperar_lista_usuarios();
      // Obtengo la lista de usuarios recibida del servidor.
      auto lista = controlador.obtener_usuarios();
      // Verifico que haya usuarios disponibles.
      if (lista.empty()) {
          printf("No hay usuarios disponibles.\n");
          continue;
      }
      // Muestro los usuarios para que el usuario pueda seleccionar uno.
      printf("Usuarios disponibles:\n");
      for (size_t i = 0; i < lista.size(); i++) {
          printf("%zu. %s [%s]\n", i + 1, lista[i].first.c_str(), lista[i].second.c_str());
      }
      // Solicito al usuario que seleccione un destinatario.
      printf("Seleccione un usuario: ");
      std::string seleccion;
      std::getline(std::cin, seleccion);
      // Convierto la selección escrita por el usuario a un número.
      int numero_usuario;
          
      try {
          numero_usuario = std::stoi(seleccion);
      } catch (...) {
          printf("Selección no válida.\n");
          continue;
      }
      // Verifico que el número corresponda a un usuario de la lista.
      if (numero_usuario < 1 || static_cast<size_t>(numero_usuario) > lista.size()) {
          printf("Selección no válida.\n");
          continue;
      }
      // Obtengo el nombre del usuario seleccionado.
      std::string destinatario = lista[numero_usuario - 1].first;
      printf("Destinatario seleccionado: %s\n", destinatario.c_str());
    }
    // Informa cuando se escribe un comando que todavía no conocemos.
    else {
        printf("Comando no reconocido.\n");
    }
  }
  return 0;
}