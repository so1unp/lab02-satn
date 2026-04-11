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

## Punto 2 — bin/fail

Al ejecutar `strace ./bin/fail` se observa que el programa intenta abrir un archivo llamado `archivo.txt` en el directorio actual:

- Si existe:
  - `openat(AT_FDCWD, "archivo.txt", O_RDONLY) = 3`
  - imprime `ok` por stdout: `write(1, "ok\n", 3)`
  - finaliza con éxito: `exit_group(0)`

- Si no existe:
  - `openat(...) = -1 ENOENT`
  - imprime `error` por stderr: `write(2, "error\n", 6)`
  - finaliza con error: `exit_group(1)`

Conclusión: el “fallo” se debe a que `archivo.txt` no está presente.  
Solución (sin acceso al código fuente): crear `archivo.txt` (por ejemplo con `touch archivo.txt`) antes de ejecutar `bin/fail`.

---

## Punto 3 - mycopy

Se implementó `bin/mycopy` con dos modos:

- `s`: usa `open`, `read`, `write`, `close`.
- `f`: usa `fopen`, `fread`, `fwrite`, `fclose`.

Además, en ambos modos:

- si el origen no existe, falla e imprime error con `perror`.
- si el destino existe, falla e imprime error con `perror`.

### Verificación funcional

Las pruebas se ejecutan en este orden para que cada escenario sea fácil de validar.

Escenario 1: origen inexistente (debe fallar)

```bash
./bin/mycopy s /tmp/no_existe_lab_999 /tmp/lab_dst_small.txt
# open src: No such file or directory
# exit code: 1

./bin/mycopy f /tmp/no_existe_lab_999 /tmp/lab_dst_small.txt
# fopen src: No such file or directory
# exit code: 1
```

Escenario 2: directorio de destino inexistente (debe fallar)

```bash
printf 'abc\n' > /tmp/lab_src_small.txt

./bin/mycopy s /tmp/lab_src_small.txt /tmp/dir_que_no_existe/out.txt
# open dst: No such file or directory
# exit code: 1

./bin/mycopy f /tmp/lab_src_small.txt /tmp/dir_que_no_existe/out.txt
# fopen dst: No such file or directory
# exit code: 1
```

Preparación para escenarios de copia correcta y destino existente:

```bash
rm -f /tmp/lab_src_small.txt /tmp/lab_dst_small.txt
printf 'abc\n' > /tmp/lab_src_small.txt
```

Escenario 3: copia correcta con destino ausente (debe funcionar)

```bash
./bin/mycopy s /tmp/lab_src_small.txt /tmp/lab_dst_small.txt
# exit code: 0

rm -f /tmp/lab_dst_small.txt
./bin/mycopy f /tmp/lab_src_small.txt /tmp/lab_dst_small.txt
# exit code: 0
```

Escenario 4: destino ya existe (debe fallar)

```bash
./bin/mycopy s /tmp/lab_src_small.txt /tmp/lab_dst_small.txt
# open dst: File exists
# exit code: 1

./bin/mycopy f /tmp/lab_src_small.txt /tmp/lab_dst_small.txt
# fopen dst: File exists
# exit code: 1
```

### Comparación con strace -c (> 1 MiB)

Se generó un archivo de 2 MiB y se ejecutó:

```bash
dd if=/dev/urandom of=/tmp/lab_big.bin bs=1M count=2 status=none
ls -lh /tmp/lab_big.bin
```

```bash
strace -c ./bin/mycopy s /tmp/lab_big.bin /tmp/lab_out_s.bin
strace -c ./bin/mycopy f /tmp/lab_big.bin /tmp/lab_out_f.bin
```

Ambas copias fueron correctas (`cmp` devolvió 0 en los dos casos).

Resumen de resultados:

- Modo `s`:
  - `read`: 8194 llamadas
  - `write`: 8192 llamadas
  - total de syscalls: 16418
  - tiempo total: 0,144894 s
- Modo `f`:
  - `read`: 514 llamadas
  - `write`: 512 llamadas
  - total de syscalls: 1063
  - tiempo total: 0,011822 s

### Diferencias observadas

En este experimento se usó `BUFFER_SIZE = 256` en el programa. Con ese tamaño, el modo `s` hace muchas más llamadas al kernel porque cada iteración dispara `read` y `write` de bloques pequeños.

En cambio, en modo `f`, la biblioteca estándar aplica buffering interno sobre los `FILE*`, por lo que termina realizando menos syscalls al kernel (bloques más grandes). Por eso el conteo de `read/write` en `strace -c` es mucho menor que en modo `s`.

Conclusión: con buffer de 256 bytes y archivo de 2 MiB, la diferencia entre ambas estrategias es evidente. El modo `f` realiza menos syscalls y tarda bastante menos tiempo total que el modo `s`.

---
