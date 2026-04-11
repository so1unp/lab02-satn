# Informe breve — Análisis con `strace` de `bin/hola`

## Comando ejecutado
Se ejecutó el programa observando la salida de `strace` por pantalla:

```bash
strace ./bin/hola
```

## Objetivo
Identificar la llamada al sistema (syscall) que termina usando `printf()` para imprimir el saludo por la salida estándar.

## Hallazgos principales
La impresión del mensaje se realiza mediante la syscall:

```text
write(1, "hola franciso\n", 14) = 14
```

**Descripción:**
- `write` es la syscall que escribe bytes en un descriptor de archivo.
- El primer argumento `1` corresponde a **stdout** (salida estándar).
- El segundo argumento es el **buffer** con el texto a imprimir.
- El tercer argumento indica la **cantidad de bytes** a escribir.
- El valor de retorno (`= 14`) indica cuántos bytes se escribieron efectivamente.

En conclusión, aunque el mensaje se imprime con `printf()` (función de biblioteca), la escritura real hacia la terminal termina ocurriendo a nivel de sistema con `write()` sobre stdout.

## Otras syscalls relevantes observadas
- Inicio del programa:
  ```text
  execve("./bin/hola", ["./bin/hola"], ...) = 0
  ```
  `execve` es la syscall que carga y ejecuta el binario.

- Finalización del programa:
  ```text
  exit_group(0) = ?
  ```
  `exit_group(0)` indica que el proceso termina correctamente (código de salida 0).

## Nota
Aparecen otras syscalls (como `open`, `mmap`, `brk`, etc.) asociadas a la carga dinámica de bibliotecas y manejo de memoria, pero no son parte directa de la impresión del saludo.

---
