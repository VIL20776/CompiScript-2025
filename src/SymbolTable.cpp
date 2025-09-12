#include <string>
#include <utility>
#include <memory>
#include <print>
#include <any>

#include "SymbolTable.h"

std::any makeAny(Symbol symbol) {
    return std::make_any<Symbol>(symbol);
}

Symbol castSymbol(std::any symbol) {
    return std::any_cast<Symbol>(symbol);
}

SymbolDataType getSymbolDataType(std::string type_name) {
    if (type_name == "integer") return SymbolDataType::INTEGER;
    if (type_name == "string") return SymbolDataType::STRING;
    if (type_name == "boolean") return SymbolDataType::BOOLEAN;
    return SymbolDataType::OBJECT;
}

std::string getSymbolDataTypeString(SymbolDataType type) {
    std::string string_type;
    switch (type) {
        case SymbolDataType::UNDEFINED: string_type = "undefined"; break;
        case SymbolDataType::INTEGER: string_type = "integer"; break;
        case SymbolDataType::BOOLEAN: string_type = "boolean"; break;
        case SymbolDataType::STRING: string_type = "string"; break;
        case SymbolDataType::OBJECT: string_type = "object"; break;
        case SymbolDataType::NIL: string_type = "null"; break;
    }
    return string_type;
}

SymbolTable::SymbolTable()
{
    global = std::make_shared<Table>();
    current = global;
    table_count = 0;
    global->id = table_count++;
}

SymbolTable::~SymbolTable()
{
}

void SymbolTable::insert(const Symbol &symbol) {
    auto new_symbol = symbol;
    new_symbol.label = "L" + std::to_string(current.lock()->id) + "_";
    current.lock()->table.emplace(symbol.name, new_symbol);
}

void SymbolTable::insert(const std::vector<Symbol> &symbols) {
    for (auto symbol: symbols)
        insert(symbol);
}

std::pair<const Symbol&, bool>
SymbolTable::lookup(const std::string &symbol_name, bool local) {
    // Try finding in current scope;
    auto exists = current.lock()->table.contains(symbol_name);
    if (current.lock()->table.contains(symbol_name))
        return {current.lock()->table.at(symbol_name), true};


    // Find in previous scope
    if (!local) {
        auto parent = current.lock()->parent.lock();
        while (parent) {
            if (parent->table.contains(symbol_name))
                return {parent->table.at(symbol_name), true};

            parent = parent->parent.lock();
        } 
    }
    return {{}, false};
}

std::pair<const Symbol&, bool> SymbolTable::get_property(const std::string &symbol_type, const std::string &property_name) {
    std::string parent = symbol_type;
    do {
        auto symbol_exists = lookup(parent, false);
        if (!symbol_exists.second) 
            return symbol_exists;

        auto symbol = symbol_exists.first;
        if (!symbol.definition.lock() || symbol.type != SymbolType::CLASS)
            return {{}, false};

        auto class_table = symbol.definition.lock();
        if (class_table->table.contains(property_name))
            return {class_table->table.at(property_name), true};

        parent = symbol.parent;
    }
    while (!parent.empty());
        

    return {{}, false};
}

bool SymbolTable::set_property(const std::string &symbol_type, const std::string &property_name, const Symbol &property_symbol) {
    // TODO: Rework on CI phase
    auto symbol_exists = lookup(symbol_type, false);
    if (!symbol_exists.second) 
        return false;

    auto symbol = symbol_exists.first;
    if (!symbol.definition.lock())
        return false;

    auto class_table = symbol.definition.lock();
    if (class_table->table.contains(property_name)) {
        class_table->table.insert_or_assign(property_name, property_symbol);
        return true;
    }

    return false;
}

bool SymbolTable::update(const std::string &symbol_name, const Symbol &symbol) {
    // Try finding in current scope
    if (current.lock()->table.contains(symbol_name)) {
        current.lock()->table.insert_or_assign(symbol_name, symbol);
        return true;
    }

    // Find in previous scope
    auto parent = current.lock()->parent.lock();
    while (parent) {
        if (parent->table.contains(symbol_name)) {
            parent->table.insert_or_assign(symbol_name, symbol);
            return true;
        }

        parent = parent->parent.lock();
    }
    
    return false;
}

void SymbolTable::addChildTable() {
    auto new_scope = std::make_shared<Table>();
    current.lock()->children.push_back(new_scope);
    new_scope->parent = current.lock();
    current = new_scope;
    current.lock()->id = table_count++;
}

void SymbolTable::setParentToCurrent() {
    current = current.lock()->parent.lock();
}

void SymbolTable::setGlobalToCurrent() {
    current = global;
}

void SymbolTable::enter() {
    int index = indexes.top();
    current = current.lock()->children.at(index);
    indexes.push(0);
}

void SymbolTable::exit() {
    setParentToCurrent();
    indexes.pop();
    indexes.top()++;
}

void SymbolTable::printTables() {
    printTable(*global.get(), "");
}

void SymbolTable::printTable(const Table& table, const std::string tabs) {
    std::println("{}Table {}:", tabs, std::to_string(table.id));
    for (auto name_symbol: table.table) {
        std::print("{}", tabs.c_str());
        printSymbol(name_symbol.second);
    }

    for (auto t: table.children) {
        printTable(*t.get(), tabs + "\t");
    }
}

void SymbolTable::printSymbol(const Symbol& symbol) {
    std::print("name=({}) ", symbol.name.c_str());
    std::print("parent=({}) ", symbol.parent.c_str());

    std::string symbol_type;
    switch (symbol.type) {
        case SymbolType::LITERAL:
            symbol_type = "Literal";
            break;
        case SymbolType::VARIABLE:
            symbol_type = "Variable";
            break;
        case SymbolType::CONSTANT:
            symbol_type = "Constant";
            break;
        case SymbolType::FUNCTION:
            symbol_type = "Function";
            break;
        case SymbolType::CLASS:
            symbol_type = "Class";
            break;
        case SymbolType::ARGUMENT:
            symbol_type = "Argument";
            break;
        case SymbolType::PROPERTY:
            symbol_type = "Property";
            break;
    }
    std::print("type=({}) ", symbol_type);

    std::string symbol_data_type;
    switch (symbol.data_type) {
        case SymbolDataType::UNDEFINED:
            symbol_data_type = "Undefined";
            break;
        case SymbolDataType::INTEGER:
            symbol_data_type = "Integer";
            break;
        case SymbolDataType::STRING:
            symbol_data_type = "String";
            break;
        case SymbolDataType::BOOLEAN:
            symbol_data_type = "Boolean";
            break;
        case SymbolDataType::NIL:
            symbol_data_type = "Null";
            break;
        case SymbolDataType::OBJECT:
            symbol_data_type = "Object";
            break;
    }
    std::print("data_type=({}) ", symbol_data_type);

    std::print("value=({}) ",symbol.value);
    std::print("arg_list=(");
    for (auto arg: symbol.arg_list) {
        std::print("{} ",arg.name);
    }
    std::print(") ");
    std::print("dimentions=({}) ",std::to_string(symbol.dimentions).c_str());
    std::print("size=({}) ",std::to_string(symbol.size).c_str());
    std::print("offset=({})\n",std::to_string(symbol.offset).c_str());

}
