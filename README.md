# Proyecto 1: Memoria Compartida: Productor - Consumidor

El propósito de este proyecto es experimentar el uso de memoria compartida entre procesos *heavyweight*.
La solución del problema es mediante la biblioteca de semáforos POSIX y POSIX memoria compartida IPC

### Estructura y compilar el proyecto
- Proyecto1: El proyecto se encuentra todo en el directorio proyecto1.
- include: Contiene los .h
- src: Son los .c de lo archivos Buffer, Creador, Productor, Consumidor y Finalizador.
- build: contiene los archivos ejecutables, generados por el makefile

Para correr el proyecto, se utiliza el archivo makefile:
```
make
```

## Correr el proyecto
Una vez que compila el proyecto se crea tres ejecutables.

### Creador
Recibe como parámetros -b buffer_name -s buffer_size

```
./build/creator -b [buffer_name] -s [buffer_size]

```

### Productor

Recibe como parámetros -b buffer_name -m exponential_mean_s

```
./build/producer -b [buffer_name] -m [exponential_mean_s]

```

### Consumidor

Recibe como parámetros -b buffer_name -m exponential_mean_s

```
./build/consumer -b [buffer_name] -m [exponential_mean_s]

```


### Finalizador

Recibe como parámetros -b buffer_name

```
./build/finalizer -b [buffer_name]

```

## Autores

- Daniel Alvarado Chou.
- Greylin Arias Montiel.
- Jorge Bolaños Solís.
- Alonso Mondal Durán.
- Kenneth Paniagua Díaz.
