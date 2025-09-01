# CompiScript 2025
Compilador de la asignatura de "Construcción de Compiladores"

### Fase de Semantica

- [ ] IDE
- [x] Tabla de símbolos
- [x] Comprobación semántica

## Como correr el programa
Se incluye un Dockerfile que construye el binario con todas sus dependencias. La ruta completa en la que se encuentra el binario en el contenedor es */app/build/cscript*.
```
docker build -t compiler-2025 .
docker run -it --rm compiler-2025
```

Ya que la IDE está pendiente, al correr la imagen de Docker se entra al contenedor en un entorno de 'bash'. Si se quieren agregar archivos de compiscript, se tendrá que usar un editor de texto de terminal (como vim o nano) o agregarlos al proyecto antes de generar el contenedor (ej. agregar el archivo .cps al directorio *example/*)
```
// Ejemplo de ejecución dentro del contenedor
./build/cscript example/program.cps
```

Se pueden ver las tablas creadas con el comando *-print-tables*. Se crea una tabla por cada ambito creado, si en el ámbito no se crearon símbolos, la tabla no tendrá entradas.
```
./build/cscript example/program.cps -print-tables
```

## Dependencias y herramientas usadas

- CMake (Build System de C++)
- gcc/g++ (Compilador de C++)
- Catch2 (Pruebas unitarias)
- Java Runtime y JDK (Dependencias de ANTLR4)
- Runtime de ANTLR4 para C++ (Dependencia para usar ANTLR4 con C++)
- JAR binario ANTLR4 (Incluido en el proyecto)
