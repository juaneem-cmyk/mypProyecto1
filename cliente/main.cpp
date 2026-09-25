#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cctype>
#include <vector>
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
      controlador.esperar_lista_usuarios();
      auto lista = controlador.obtener_usuarios();
      printf("Usuarios disponibles:\n");
      for (size_t i = 0; i < lista.size(); i++) {
          printf("%zu. %s [%s]\n", i + 1, lista[i].first.c_str(), lista[i].second.c_str());
      }
    }

    // Solicita al servidor la creación de una nueva sala
    else if (comando == "NEW_ROOM") {
      printf("Ingrese el nombre de la sala: ");    
      std::string nombre_sala;
      std::getline(std::cin, nombre_sala);
      
      if (nombre_sala.empty() || nombre_sala.size() > 16) {
        printf("Nombre de sala no válido.\n");
        continue;
      }
      std::string mensaje =
      controlador.crear_nueva_sala(nombre_sala);
      cliente.enviar_mensaje(mensaje);
    }

    // Solicita invitar uno o varios usuarios a una sala
    else if (comando == "INVITE") {
      printf("Ingrese el nombre de la sala: ");
      std::string nombre_sala;
      std::getline(std::cin, nombre_sala);

      if (nombre_sala.empty() || nombre_sala.size() > 16) {
          printf("Nombre de sala no válido.\n");
          continue;
      }
      printf("¿Cuántos usuarios desea invitar?: ");
      std::string entrada_cantidad;
      std::getline(std::cin, entrada_cantidad);
      size_t cantidad_usuarios;

      try {
          cantidad_usuarios = std::stoul(entrada_cantidad);
      } catch (...) {
          printf("Cantidad no válida.\n");
          continue;
      }

      if (cantidad_usuarios == 0) {
          printf("Debe invitar al menos a un usuario.\n");
          continue;
      }
      std::vector<std::string> usuarios;
      // Solicita el nombre de cada usuario que será invitado
      for (size_t i = 0; i < cantidad_usuarios; i++) {
          printf("Ingrese el usuario %zu: ", i + 1);
          std::string usuario;
          std::getline(std::cin, usuario);

          if (usuario.empty() || usuario.size() > 8) {
              printf("Nombre de usuario no válido.\n");
              usuarios.clear();
              break;
          }
          usuarios.push_back(usuario);
      }

      if (usuarios.size() != cantidad_usuarios) {
          continue;
      }
      std::string mensaje = controlador.crear_invitacion(nombre_sala, usuarios);
      cliente.enviar_mensaje(mensaje);
  }

    // Cambia el estado del usuario
    else if (comando == "STATUS") {
      printf("Ingrese el nuevo estado (ACTIVE, AWAY o BUSY): ");
      std::string estado;
      std::getline(std::cin, estado);

      // Verifico que el estado sea uno de los permitidos
      if (estado != "ACTIVE" && estado != "AWAY" && estado != "BUSY") {
        printf("Estado no válido.\n");
        continue;
      }
      std::string mensaje_status = controlador.crear_status(estado);
      cliente.enviar_mensaje(mensaje_status);
      printf("Estado cambiado a %s.\n", estado.c_str());
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
      printf("Destinatario seleccionado: %s\n", destinatario.c_str());
      // Verifico que el usuario no haya seleccionado su propio nombre :v
      if (destinatario == usuario) {
        printf("No puedes enviarte un mensaje a ti mismo.\n");
        continue;
      }
      printf("Escribe tu mensaje: ");
      std::string texto;
      std::getline(std::cin, texto);
      std::string mensaje_texto = controlador.crear_texto(destinatario, texto);
      cliente.enviar_mensaje(mensaje_texto);
    }
  }
  return 0;
}