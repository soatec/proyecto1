# Proyecto 1: Memoria Compartida: Productor - Consumidor

El proposito de este proyecto es experimentar el uso de memoria compartida entre proceso *heavyweight*.
La solución del problema es mediante la libreía de semaforos POSIX y POSIX memoria compartida IPC

### Estructura y compilar el proyecto
- Proyecto1: El proyecto se encuentra todo el la carpeta Proyecto1.
- include: Contiene los .h
- src: Son los .c de lo archivos Buffer, Creator, Productor, Consumidor y Finalizador.
- buils: contiene los archivos ejecutables, generados por el makefile

Para correr el proyecto, se utiliza el archivo makefile:
```
make
```

## Correr el proyecto
Una vez que compila el proyecto se crea tres ejecutables.

### Creador
Resive como parametros -b buffer_name -c buffer_size

```
./build/creator -b [buffer_name] -c [buffer_size]

```

### Productor

Resive como parametros -b buffer_name -m exponential_mean_s

```
./build/producer -b [buffer_name] -m [exponential_mean_s]

```

### Consumidor

Resive como parametros -b buffer_name -m exponential_mean_s

```
./build/consumer -b [buffer_name] -m [exponential_mean_s]

```

### Finalizador

Resive como parametros -b buffer_name

```
./build/finalizer -b [buffer_name]

```

## Auttores

- Daniel Alvarado Chou.
- Greylin Arias Montiel.
- Jorge Bolaños Solís.
- Alonso Mondal Durán.
- Kenneth Paniagua Díaz.
