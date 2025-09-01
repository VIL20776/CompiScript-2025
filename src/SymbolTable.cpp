#include <string>
#include <utility>
#include <memory>
#include <print>

#include "SymbolTable.h"

SymbolTable::SymbolTable()
{
    global = std::make_shared<Table>();
    current = global;
}

SymbolTable::~SymbolTable()
{
}

void SymbolTable::insert(const Symbol &symbol) {
    current.lock()->table.emplace(symbol.name, symbol);
}

// std::vector<Symbol>
// SymbolTable::find_range(const std::string &label) {
    // std::vector<Symbol> symbols;
    // for (auto &scope: scopes)
    // for (auto &kv : scope) 
    // {
    //     auto &key = kv.first;
    //     if (key.second == label)
    //         symbols.push_back(kv.second);
    // }
    //
    // return symbols;
// }

std::pair<const Symbol&, bool>
SymbolTable::lookup(const std::string &symbol_name, bool local) {
    // Try finding in current scope;

    if (!current.lock()) {
        std::println("current is null");
    }

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
    std::string label = symbol_type;
    do {
        auto symbol_exists = lookup(label, false);
        if (!symbol_exists.second) 
            return symbol_exists;

        auto symbol = symbol_exists.first;
        if (!symbol.definition.lock() || symbol.type != SymbolType::CLASS)
            return {{}, false};

        auto class_table = symbol.definition.lock();
        if (class_table->table.contains(property_name))
            return {class_table->table.at(property_name), true};

        label = symbol.label;
    }
    while (!label.empty());
        

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

void SymbolTable::enter(const std::vector<Symbol> &initial_symbols) {
    auto new_scope = std::make_shared<Table>();
    for (auto symbol: initial_symbols) 
        new_scope->table.insert_or_assign(symbol.name, symbol);

    current.lock()->children.push_back(new_scope);
    new_scope->parent = current.lock();
    current = new_scope;
}

void SymbolTable::exit() {
    current = current.lock()->parent.lock();
}

void SymbolTable::printTables() {
    int id = 0;
    printTable(*global.get(), "", id);
}

void SymbolTable::printTable(const Table& table, const std::string tabs, int& id) {
    std::println("{}Table {}:", tabs, std::to_string(id));
    for (auto name_symbol: table.table) {
        std::print("{}", tabs.c_str());
        printSymbol(name_symbol.second);
    }

    for (auto t: table.children) {
        printTable(*t.get(), tabs + "\t", ++id);
    }
}

void SymbolTable::printSymbol(const Symbol& symbol) {
    std::print("name=({}) ", symbol.name.c_str());
    std::print("label=({}) ", symbol.label.c_str());

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
