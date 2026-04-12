# Informe breve — Análisis con `strace` de `bin/hola`

## Comando ejecutado

Se ejecutó `bin/hola` con `strace` en ambos caminos (`con` y `sin` argumento):

```bash
strace -e execve,write,exit_group ./bin/hola
strace -e execve,write,exit_group ./bin/hola miyen
```

## Objetivo

Verificar qué syscall usa `printf()` para imprimir y cómo cambia la salida según `argv[1]`.

## Hallazgos principales

En ambos casos, la impresión termina en `write` sobre `stdout` (`fd=1`):

```text
write(1, "Hola desconocido\n", 17) = 17
write(1, "Hola miyen\n", 11) = 11
```

## Otras syscalls relevantes observadas

- Con argumento:

```text
execve("./bin/hola", ["./bin/hola", "miyen"], ...) = 0
```

- Finalización del programa:

```text
exit_group(0) = ?
```

## Nota

Comportamiento observado: si no hay argumento imprime el valor por defecto; si hay argumento imprime `argv[1]`.

---

## Punto 2 — bin/fail

Con `strace`, `bin/fail` intenta abrir `archivo.txt` en el directorio actual.

- Caso archivo presente:
  - `openat(AT_FDCWD, "archivo.txt", O_RDONLY) = 3`
  - `write(1, "ok\n", 3)`
  - `exit_group(0)`

- Caso archivo ausente:
  - `openat(AT_FDCWD, "archivo.txt", O_RDONLY) = -1 ENOENT`
  - `write(2, "error\n", 6)`
  - `exit_group(1)`

Conclusión: la falla se debe a la ausencia de `archivo.txt` en el directorio de ejecución. Con `archivo.txt` presente, `bin/fail` finaliza correctamente.

---

## Punto 3 - mycopy

Se implementó `bin/mycopy` con dos modos:

- `s`: usa `open`, `read`, `write`, `close`.
- `f`: usa `fopen`, `fread`, `fwrite`, `fclose`.

Además, en ambos modos:

- si el origen no existe, falla e imprime error con `perror`.
- si el destino existe, falla e imprime error con `perror`.

### Verificación funcional

Se validaron cuatro escenarios en ambos modos (`s` y `f`).

| Escenario | Resultado esperado | Resultado observado |
| --- | --- | --- |
| Origen inexistente | Falla con exit code 1 | Cumple en `s` y `f` |
| Directorio de destino inexistente | Falla con exit code 1 | Cumple en `s` y `f` |
| Destino ausente | Copia correcta con exit code 0 | Cumple en `s` y `f` |
| Destino existente | Falla con `File exists` y exit code 1 | Cumple en `s` y `f` |

En todos los casos, ambos modos mostraron el comportamiento esperado.

Bloque mínimo reproducible (caso: destino existente, debe fallar):

```bash
touch /tmp/origen.txt
touch /tmp/destino.txt
./bin/mycopy s /tmp/origen.txt /tmp/destino.txt
# esperado: open dst: File exists
# exit code: 1
```

### Comparación con strace -c (> 1 MiB)

Se generó un archivo de 2 MiB y se ejecutó:

Forma simple y rápida de crear el archivo de prueba:

```bash
truncate -s 2M /tmp/lab_big.bin
ls -lh /tmp/lab_big.bin
```

Pruebas con strace sobre dicho archivo
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
  - tiempo total: 0,110315 s
- Modo `f`:
  - `read`: 514 llamadas
  - `write`: 512 llamadas
  - total de syscalls: 1063
  - tiempo total: 0,011781 s

Nota: los tiempos pueden variar entre ejecuciones, pero la relación entre ambos modos se mantiene.

### Diferencias observadas

Para esta comparación se usó `BUFFER_SIZE = 256`.

- Con `BUFFER_SIZE = 1`, la prueba se vuelve demasiado lenta por la gran cantidad de iteraciones y syscalls.
- Con `BUFFER_SIZE = 4096`, para un archivo de 2 MiB la diferencia entre modos se vuelve poco significativa.
- Con `BUFFER_SIZE = 256`, la diferencia queda bien visible en `strace -c`: el modo `s` concentra muchas más llamadas `read/write`, mientras que el modo `f` (por buffering interno de `stdio`) reduce fuertemente esa cantidad.

Conclusión: `BUFFER_SIZE = 256` fue un punto intermedio útil para observar el contraste entre ambos enfoques sin que la prueba sea excesivamente lenta.

---
