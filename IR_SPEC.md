# Especificacion del Codigo Intermedio
### Variables
Las variables pueden contener un valor entero o una direccion en memoria. Si la variable tiene como sufijo el operador "*" se interpreta que se esta apuntando a una direccion de memoria.

#### Tipos de variables
* tn: Variables temporales que se usan para operaciones intermedias
* i: Variable temporal usada durante operaciones de indexado
* ret: Variable que contiene el valor de retorno de una subrutina
* err: Variable usada por el operador `iferr` para comprobar errores de ejecucion. Si el error ocurre dentro de un `try-catch`, se almacena aqui un puntero al mensaje de error.
* catch: Se almacena aqui la direccion en memoria de una subrutina de manejo de errores.
* switch: Aqui se almacena el valor usado las comparaciones en un `switch-case` 
* case: Almacena la direccion del bloque de codigo a ejecutar.

### Operaciones
Las operaciones siguen la siguente sintaxis:

`op arg1 arg2`

Las asignaciones pueden darse de la siguiente forma:
```
res = arg
res = op arg1 arg2
```

#### Operadores
* Operadores aritmaticos: "+, -, *, /, %"
* Operadores coparativos: ">, >=, <, <=, ==, !="
* Operadores logicos: "&&, ||"
* Operadores de secuencia: "goto, if, ifnot, iferr"
* Operadores de marcado: "tag"
* Operadores de subrutinas: "begin, end, call, push, pop, return"
* Operadores de cadenas: "to_str, concat, streql, strneq"
* Operadores de memoria: "alloc"

`goto` toma como argumento una etiqueta que representa la direccion en memoria a la que se tiene que saltar.  
`if` salta a la direccion en memoria del segundo argumento si el valor del primer argumento es verdadero (diferente de 0).  
`ifnot` salta a la direccion en memoria del segundo argumento si el valor del primer argumento no es verdadero (igual a 0).  
`iferr` salta a una subrutina de manejo de error si el valor en de `err` es diferente de 0. Si el valor de `catch` es diferente de 0, salta a la direccion de memoria almacenada en la variable en su lugar.  
`tag` crea una etiqueta que representa la direccion en memoria donde fue declarada. Usadas para control de flujo y bucles.  
`begin` y `end` crean una etiqueta que representa la direccion en memoria donde fue declarada e indican el inicio y el final de la definicion de una subrutina.  
`call` guarda la direccion en memoria de la siguiente instruccion y salta a la subrutina indicada en el primer argumento.  
`push` guarda una valor en el stack.  
`pop` saca un valor del stack.  
`return` devuelve el programa a la direccion en memoria previa a la llamada de la subrutina. Si recive una variable como argumento, devuelve el valor de la variable al registro de destino.  
`to_str` convierte el valor numerico a una representacion ASCII guardada en memoria dinamica.  
`concat` combina dos cadenas y el resultado es almacenado en memoria dinamica.  
`streql` y `strneq` comparan si dos cadenas son iguales y retornan un 1 y 0 respectivamente si se cumple la condicion.  
`alloc` reserva una determinada cantidad de memoria dinamica y devuelve un puntero a esa memoria.  

