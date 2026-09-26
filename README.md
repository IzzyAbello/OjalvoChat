# OjalvoChat
Primer Proyecto de Modelado y Programación CS-FC-UNAM

Autor: Isidro A. Abelló García

Profesor: Dr. Canek Pelaez Valdes

Servidor y cliente de un chat vía TCP.

## Requisitos

Se requieren las siguientes bibliotecas:
* Threads
* G-Lib 2.0
* cJSON
* Criterion (sólo para ejecutar tests del servidor)
* .NET 10.0

## Comandos del Servidor

### Instalar

``` bash/fish
meson setup build
```

### Reinstalar

``` bash/fish
meson setup --reconfigure build
```

### Compilar servidor
``` bash/fish
meson compile -C build
```

### Tests de Meson + Criterion

``` bash/fish
meson compile -C build check
```

### Ejecutar Server

``` bash/fish
./build/src/ojalvochat
```

## Comandos del Cliente

### Instalar

```bash/fish
dotnet build
```

### Ejecutar

```bash/fish
dotnet run [PUERTO] [DIRECCIÓN IP]
```