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

std::pair<const Symbol&, bool> SymbolTable::get_property(const std::string &symbol_name, const std::string &property_name) {
    auto symbol_exists = lookup(symbol_name, false);
    if (!symbol_exists.second) 
        return symbol_exists;

    auto symbol = symbol_exists.first;
    if (!symbol.definition.lock())
        return {{}, false};

    auto class_table = symbol.definition.lock();
    if (class_table->table.contains(property_name))
        return {class_table->table.at(property_name), true};

    return {{}, false};
}

bool SymbolTable::set_property(const std::string &symbol_name, const std::string &property_name, const Symbol &property_symbol) {
    auto symbol_exists = lookup(symbol_name, false);
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
