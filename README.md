# Chat TCP

Proyecto de chat cliente-servidor desarrollado para la asignatura de **Modelado y Programación** de la Facultad de Ciencias, UNAM.

El proyecto implementa un sistema de comunicación mediante **TCP**, con un servidor desarrollado en **C** y un cliente desarrollado en **C++**. La comunicación entre ambos utiliza mensajes **JSON**, siguiendo un protocolo definido para manejar usuarios, estados, mensajes privados, mensajes públicos y salas.

## Características

* Comunicación mediante sockets TCP.
* Servidor desarrollado en C.
* Cliente desarrollado en C++.
* Intercambio de mensajes mediante JSON.
* Manejo de múltiples clientes utilizando `poll()`.
* Identificación de usuarios.
* Estados de usuario:

  * `ACTIVE`
  * `AWAY`
  * `BUSY`
* Consulta de usuarios conectados.
* Mensajes privados entre usuarios.
* Mensajes públicos.
* Creación de salas privadas.
* Invitaciones a salas.
* Unión y salida de salas.
* Consulta de usuarios dentro de una sala.
* Mensajes dentro de salas.
* Desconexión controlada de usuarios.
* Recepción de mensajes en el cliente mediante un hilo independiente.
* Construcción y análisis de JSON mediante `json-c`.
* Manejo de usuarios y salas mediante estructuras de GLib.

## Protocolo

Los mensajes intercambiados entre cliente y servidor utilizan JSON y terminan con un salto de línea (`\n`), que funciona como delimitador entre mensajes.

### Operaciones

| Operación     | Descripción                                        |
| ------------- | -------------------------------------------------- |
| `IDENTIFY`    | Identifica al usuario ante el servidor.            |
| `STATUS`      | Cambia el estado del usuario.                      |
| `USERS`       | Solicita la lista de usuarios conectados.          |
| `TEXT`        | Envía un mensaje privado.                          |
| `PUBLIC_TEXT` | Envía un mensaje público.                          |
| `NEW_ROOM`    | Crea una nueva sala.                               |
| `INVITE`      | Invita usuarios a una sala.                        |
| `JOIN_ROOM`   | Permite unirse a una sala mediante una invitación. |
| `ROOM_USERS`  | Solicita los usuarios de una sala.                 |
| `ROOM_TEXT`   | Envía un mensaje dentro de una sala.               |
| `LEAVE_ROOM`  | Abandona una sala.                                 |
| `DISCONNECT`  | Desconecta al usuario del servidor.                |

Además, el servidor puede enviar eventos como `NEW_USER`, `NEW_STATUS`, `USER_LIST`, `TEXT_FROM`, `PUBLIC_TEXT_FROM`, `INVITATION`, `JOINED_ROOM`, `ROOM_USER_LIST`, `ROOM_TEXT_FROM`, `LEFT_ROOM`, `DISCONNECTED` y `RESPONSE`.

## Tecnologías

* **C**
* **C++**
* **TCP / POSIX Sockets**
* **JSON**
* **json-c**
* **GLib**
* **poll()**
* **std::thread**
* **std::mutex**
* **std::condition_variable**
* **Meson**
* **Ninja**

## Requisitos

Para compilar el proyecto se necesitan:

* GCC / G++
* Meson
* Ninja
* json-c
* GLib

El proyecto está pensado para sistemas compatibles con sockets POSIX.

## Compilación

Desde el directorio raíz del proyecto:

```bash
meson setup build
meson compile -C build
```

Si ya existe el directorio `build`, basta con recompilar:

```bash
meson compile -C build
```

## Ejecución

Primero se inicia el servidor:

```bash
./build/servidor/servidor
```

Después, desde otra terminal, se ejecuta el cliente:

```bash
./build/cliente/cliente
```

El servidor utiliza:

```text
127.0.0.1:1234
```

por defecto para la ejecución local.

Se pueden ejecutar varios clientes para probar la comunicación entre usuarios.

## Organización

La estructura principal del proyecto se divide en servidor y cliente:

```text
.
├── servidor/
│   ├── main.c
│   ├── cliente.c
│   ├── cliente.h
│   ├── sala.c
│   ├── sala.h
│   ├── protocolo.c
│   ├── protocolo.h
│   ├── comandos.c
│   └── comandos.h
│
├── cliente/
│   ├── main.cpp
│   ├── Controlador.cpp
│   ├── Controlador.h
│   ├── TCPCliente.cpp
│   └── TCPCliente.h
│
├── meson.build
└── README.md
```

## Límites

El protocolo establece límites para algunos elementos del sistema:

* Nombre de usuario: máximo **8 caracteres**.
* Nombre de sala: máximo **16 caracteres**.
* Mensaje: máximo **1 MiB**.
