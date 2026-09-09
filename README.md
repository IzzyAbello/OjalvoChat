# OjalvoChat
Primer Proyecto MyP-CS-FC-UNAM

## Comandos

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

