#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

enum class SymbolType: int {
    LITERAL,
    VARIABLE,
    ARGUMENT,
    FUNCTION,
    CLOUSURE,
    CLASS,
    ARRAY
};

enum class SymbolDataType: int {
    UNDEFINED,
    INTEGER,
    BOOLEAN,
    STRING,
    OBJECT,
    NIL,
};

/*
name - Identificacion
label - Etiqueta identificadora:
    Si es VARIABLE de tipo de dato OBJECT, indica la clase de la que es instancia.
    Si es FUNCTION, indica a cual clase pertenece.
    Si es CLASS, indica de que clase hereda
type - Tipo de simbolo
data_type - Tipo de dato. 
value - Valor contenido en la variable.
arg_list - Lista de argumentos para una funcion o cerradura.
prop_list - Lista de propiedades de una clase u objeto.
size - Tamaño del símbolo
offset - ubicacion en memoria

*/
struct Symbol {
    std::string name;
    std::string label;
    SymbolType type;
    SymbolDataType data_type; 
    std::string value;
    std::vector<std::string> arg_list;
    std::vector<std::string> prop_list;
    int size;
    int offset;
};

class SymbolTable
{
private:

    struct Table { 
        std::weak_ptr<Table> parent;
        std::vector<std::shared_ptr<Table>> children;
        std::unordered_map<std::string, Symbol> table;
    };

    std::shared_ptr<Table> current;
    
public:
    SymbolTable();
    ~SymbolTable();

    void insert(const Symbol &symbol);

    std::pair<const Symbol&, bool> lookup(const std::string &symbol_name);

    bool update(const std::string &symbol_name, const Symbol &symbol);

    void enter(const std::vector<Symbol> &initial_symbols = {});
    void exit();

};
