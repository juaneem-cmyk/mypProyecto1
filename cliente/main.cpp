#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cctype>
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
    // Permite cerrar el cliente desde la consola
    if (comando == "DISCONNECT") {
        std::string mensaje_desconexion = controlador.crear_desconexion();
        cliente.enviar_mensaje(mensaje_desconexion);
        break;
    }

    // Solicita la lista de usuarios al servidor
    if (comando == "USERS") {
        std::string mensaje = controlador.crear_solicitud_usuarios();
        cliente.enviar_mensaje(mensaje);
    }

    // Inicia el proceso para enviar un mensaje privado
    else if (comando == "TEXT") {
      std::string mensaje = controlador.crear_solicitud_usuarios();
      cliente.enviar_mensaje(mensaje);
      controlador.esperar_lista_usuarios();

      // Obtengo la lista de usuarios recibida del servidor
      auto lista = controlador.obtener_usuarios();
      
      // Verifico que haya usuarios disponibles
      if (lista.empty()) {
          printf("No hay usuarios disponibles.\n");
          continue;
      }
      // Muestro los usuarios para que el usuario pueda seleccionar uno
      printf("Usuarios disponibles:\n");
      for (size_t i = 0; i < lista.size(); i++) {
          printf("%zu. %s [%s]\n", i + 1, lista[i].first.c_str(), lista[i].second.c_str());
      }
      // Solicito al usuario que seleccione un destinatario
      printf("Seleccione un usuario por número o nombre: ");
      std::string seleccion;
      std::getline(std::cin, seleccion);
      std::string destinatario;
      bool usuario_encontrado = false;
      // Verifico si la selección está formada solamente por números
      bool es_numero = !seleccion.empty() && std::all_of(seleccion.begin(), seleccion.end(), [](unsigned char caracter) {return std::isdigit(caracter);});
              
      if (es_numero) {
        // Convierto la selección numérica a un índice de la lista
        try {
          int numero_usuario = std::stoi(seleccion);
          // Verifico que el número corresponda a un usuario.
          if (numero_usuario >= 1 && static_cast<size_t>(numero_usuario) <= lista.size()) {
            destinatario = lista[numero_usuario - 1].first;
            usuario_encontrado = true;
          }
        } catch (...) {
          usuario_encontrado = false;
        }
      } else {
        // Busco directamente el nombre escrito por el usuario
        for (const auto& usuario : lista) {
          if (usuario.first == seleccion) {
            destinatario = usuario.first;
            usuario_encontrado = true;
            break;
          }
        }
      }
      // Verifico que el usuario seleccionado exista en la lista
      if (!usuario_encontrado) {
        printf("Selección no válida.\n");
        continue;
      }
      printf("Destinatario seleccionado: %s\n",
      destinatario.c_str());
    }
  }
  return 0;
}