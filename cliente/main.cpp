#include <iostream>
#include "TCPCliente.h"

/*Esto lo iba a colocar en el reporte pero supongo que aquí también es importante,
  estoy leyendo un libro que se llama "Como programar en C/C++ y Java" de la editorial Prentice Hall 
  tiene unas hormigas en la portada :D*/

int main() {
  TCPCliente cliente("127.0.0.1", 1234);
  if (!cliente.conectar()) {
    return 1;
  }
  std::cout << "Cliente conectado al servidor." << std::endl;
  std::cin.get();
  std::cin.get();

  return 0;
}