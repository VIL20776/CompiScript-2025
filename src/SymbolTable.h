#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

enum class SymbolType: int {
    LITERAL,
    VARIABLE,
    CONSTANT,
    ARGUMENT,
    FUNCTION,
    PROPERTY,
    CLASS,
};

enum class SymbolDataType: int {
    UNDEFINED,
    INTEGER,
    BOOLEAN,
    STRING,
    OBJECT,
    NIL,
};

struct Table;
/*
name - Identificacion
parent - Etiqueta identificadora:
    Si es VARIABLE de tipo de dato OBJECT, indica de que clase es instancia.
    Si es CLASS, indica de que clase hereda
type - Tipo de simbolo
data_type - Tipo de dato. 
value - Valor contenido en la variable.
arg_list - Lista de argumentos para una funcion o cerradura.
definition - Exclusivo de clases y funciones, contiene una referencia a la tabla de simbolos
    donde fue definida la funcion o la clase.
size - Tamaño del símbolo, si es un array, indica el tamaño del array
dimentions - Dimensiones del array, 0 si es un tipo de dato normal
offset - ubicacion en memoria

*/
struct Symbol {
    std::string name;
    std::string label;
    SymbolType type;
    SymbolDataType data_type; 
    std::string value;
    std::vector<Symbol> arg_list;
    std::weak_ptr<Table> definition;
    int size = 0;
    int dimentions = 0;
    int offset = 0;
};

struct Table { 
    std::weak_ptr<Table> parent;
    std::vector<std::shared_ptr<Table>> children;
    std::unordered_map<std::string, Symbol> table;
};

class SymbolTable
{
private:

    std::shared_ptr<Table> global;
    std::weak_ptr<Table> current;
    
public:
    SymbolTable();
    ~SymbolTable();

    void insert(const Symbol &symbol);

    std::pair<const Symbol&, bool> lookup(const std::string &symbol_name, bool local = true);

    std::pair<const Symbol&, bool> get_property(const std::string &symbol_name, const std::string &property_name);

    bool set_property(const std::string &symbol_name, const std::string &property_name, const Symbol &symbol);

    bool update(const std::string &symbol_name, const Symbol &symbol);

    void enter(const std::vector<Symbol> &initial_symbols = {});
    void exit();

    std::weak_ptr<Table> getCurrent() {return current;}
};
