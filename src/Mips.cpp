#include <regex>
#include <print>
#include <string>
#include <stack>
#include <set>

#include "Mips.h"
#include "SymbolTable.h"

using namespace CompiScript;

Mips::Mips(const std::vector<Quad> &quadruplets): quadruplets(quadruplets) {}

std::string Mips::generateDataSection() {
    std::string data_section;

    auto string_regex = std::regex("\"([^\"\r\n])*\"");
    auto int_regex = std::regex("[0-9]+");

    int string_count = 0;
    std::set<std::string> variables;
    for (int i = 0; i < quadruplets.size(); i++) {
        auto &quad = quadruplets.at(i);
        // Handle strings
        if (std::regex_match(quad.arg1, string_regex)) {
            auto string_var = "str" + std::to_string(string_count++);
            auto string_declaration = string_var + ":\t\t.asciiz\t" + quad.arg1 + "\n";
            quad.arg1 = string_var;
            data_section += string_declaration;
        }

        if (std::regex_match(quad.arg2, string_regex)) {
            auto string_var = "str" + std::to_string(string_count++);
            auto string_declaration = string_var + ":\t\t.asciiz\t" + quad.arg2 + "\n";
            quad.arg2 = string_var;
            data_section += string_declaration;
        }

        // Handle variables
        if (quad.result.empty() || variables.contains(quad.result)) continue;

        std::string var_declaration;
        if (quad.result.starts_with("W")) {
            var_declaration = quad.result + ":\t\t.word\t";
            if (std::regex_match(quad.arg1, int_regex)) {
                var_declaration += quad.arg1 + "\n";
                quadruplets.erase(quadruplets.begin() + i);
            }
            else var_declaration += "0\n";
        }
        if (quad.result.starts_with("B")) {
            var_declaration = quad.result + ":\t\t.byte\t";
            if (quad.arg1 == "false" || quad.arg1 == "null") {
                var_declaration += "0\n";
                quadruplets.erase(quadruplets.begin() + i);
            } else if (quad.arg1 == "true") {
                var_declaration += "1\n";
                quadruplets.erase(quadruplets.begin() + i);
            } 
            else var_declaration += "0\n";
        }
        if (quad.result.starts_with("S")) {
            auto storage_type = (quad.op == "alloc") ? ":\t\t.space\t"+ quad.arg1: ":\t\t.word\t0";
            if (quad.op == "alloc") {
                var_declaration = quad.result + ":\t\t.space\t"+ quad.arg1 + "\n";
                quadruplets.erase(quadruplets.begin() + i);
            }
            else var_declaration = quad.result + ":\t\t.word\t0" + "\n";
        }

        variables.insert(quad.result);
        data_section += var_declaration;
    }

    return data_section;
}

Register Mips::spill_or_assign(const std::string &var) {
    auto var_is_integer = std::regex_match(var, std::regex("[0-9]+"));
    std::array<std::string, 8> *registers = (var.contains("_")) ? &saved: &temporaries;
    std::string reg_type = (var.contains("_")) ? "$s": "$t";

    // Check if a backup exists
    for (int i = 0; i < registers->size(); i++) {
        auto reg_value = &registers->at(i);
        if (variables.count(*reg_value) > 1) {
            auto range = variables.equal_range(registers->at(i));
            auto reg = reg_type + std::to_string(i);
            std::string inst = 
                (var_is_integer) ? "li ": 
                (var.starts_with("str") || var.starts_with("S")) ? "la ": 
                (var.starts_with("B")) ? "lb": "lw ";
            std::string text = inst + reg + ", " + var;

            registers->at(i) = var;
            if (!var_is_integer) variables.insert({var, reg});
            for (auto it = range.first; it != range.second; it++) {
                if (it->second == reg) {
                    variables.erase(it);
                    break;
                }
            }

            return {reg, text + "\n"};
        }
    }

    return {"", ""};
}

Register Mips::getRegister(const std::string &var) {
    if (var.empty() || var.starts_with("l") || var.starts_with("err_")) return Register{};

    if (var == "i" || var == "err" || var == "switch") return Register("$t8","");
    if (var == "catch" || var == "case") return Register("$t9","");
    if (var.starts_with("i*")) return Register("($t8)","");
    if (var == "ret") return Register{"$v0", ""};
    if (var == "p") return Register{"$v1", ""};
    // Find var in register descriptors
    for (int i = 0; i < temporaries.size(); i++) {
        if (var == temporaries.at(i))
            return {"$t" + std::to_string(i), ""};
    }

    for (int i = 0; i < saved.size(); i++) {
        if (var == saved.at(i))
            return {"$s" + std::to_string(i), ""};
    }

    for (int i = 0; i < args.size(); i++) {
        if (var == args.at(i))
            return {"$a" + std::to_string(i), ""};
    }

    // find empty register in apropiate descriptor
    auto is_integer = std::regex_match(var, std::regex("[0-9]+"));
    std::array<std::string, 8> *registers = (var.contains("_")) ? &saved: &temporaries;
    std::string reg_type = (var.contains("_")) ? "$s": "$t";

    for (int i = 0; i < registers->size(); i++) {
        if (registers->at(i).empty()) {
            auto reg = reg_type + std::to_string(i);
            std::string inst = 
                (is_integer) ? "li ": 
                (var.starts_with("str") || var.starts_with("S")) ? "la ": 
                (var.starts_with("B")) ? "lb ": "lw ";
            std::string text = inst + reg + ", " + var;

            registers->at(i) = var;
            if (!is_integer) variables.insert({var, reg});

            return {reg, text + "\n"};
        }
    }

    return spill_or_assign(var);
}

std::string Mips::generateTextSection() {
    std::stack<std::string> subrutine_sections;
    std::string text_section = "main:\n";
    int arg_count = 0;
    int err_labels = 0;
    for (auto quad: quadruplets) {
        if (quad.result == "t0" && !(quad.arg1 == "t0" || quad.arg2 == "t0"))
            for (auto &reg: temporaries) if (reg.starts_with("t")) reg.clear(); 

        if (quad.op == "arg") {
            args.at(arg_count++) = quad.arg1;
            continue;
        }
        if (quad.op == "param") {
            auto ry = getRegister(quad.arg1);
            auto arg_reg = "$a" + std::to_string(arg_count++);
            text_section += ry.text + "move " + arg_reg + ", " + ry.reg + "\n";
            continue;
        }
        if (arg_count > 0) arg_count = 0;

        if (quad.op == "tag") {
            text_section += quad.arg1 + ":\n";
            continue;
        }

        if (quad.op == "begin") {
            subrutine_sections.push(text_section);
            text_section.clear();
            text_section += quad.arg1 + ":\n";
            continue;
        }

        if (quad.op == "end") {
            if (!text_section.ends_with("jr $ra\n\n")) text_section += "jr $ra\n\n";
            text_section += subrutine_sections.top();
            subrutine_sections.pop();
            continue;
        }
        if (quad.op == "call") {
            text_section += "addi $sp, -4\n";
            text_section += "sw $ra, ($sp)\n";
            text_section += "jal " + quad.arg1 + "\n";
            text_section += "lw $ra, ($sp)\n";
            text_section += "addi $sp, 4\n";
            continue;
        }
        if (quad.op == "goto") {
            text_section += "b " + quad.arg1 + "\n";
            continue;
        }
        if (quad.op == "print") {
            text_section += "addi $sp, -4\n";
            text_section += "sw $a0, ($sp)\n";
            text_section += "move $a0, $v1\n";
            text_section += "li $v0, 4\n";
            text_section += "# syscall\n";
            text_section += "lw $a0, ($sp)\n";
            text_section += "addi $sp, 4\n";
            text_section += "move $zero, $v0\n";
            text_section += "move $zero, $v1\n";
            continue;
        }

        auto ry = getRegister(quad.arg1);
        auto rz = getRegister(quad.arg2);
        auto rx = getRegister(quad.result);
        auto op = quad.op;

        text_section += ry.text + rz.text;
        if (op.empty()) {
            if (rx.reg.starts_with("(")) {
                std::string inst = (quad.result.starts_with("*b")) ? "sb ": "sw ";
                text_section += inst + ry.reg + ", " + rx.reg + "\n";
            } else if (ry.reg.starts_with("(")) {
                std::string inst = (quad.arg1.starts_with("*b")) ? "lb ": "lw ";
                text_section += inst + rx.reg + ", " + ry.reg + "\n";
            }
            else {
                text_section += "move " + rx.reg + ", " + ry.reg + "\n";
                if (rx.reg.starts_with("$s")) {
                    std::string inst = (quad.result.starts_with("B")) ? "sb ": "sw ";
                    text_section += inst + rx.reg + ", " + quad.result + "\n";
                }
            }
        }
        if (op == "return") {
            text_section += "move $v0, " + ry.reg + "\n";
            text_section += "jr $ra\n\n";
        }
        if (op == "if") {
            text_section += "bne $zero, " + ry.reg + ", " + quad.arg2 +"\n";
        }
        if (op == "ifnot") {
            text_section += "beq $zero, " + ry.reg + ", " + quad.arg2 +"\n";
        }
        if (op == "iferr") {
            text_section += "beq $zero, $t8, no_err" + std::to_string(err_labels) + "\n";
            text_section += "# beq $zero, $t9, " + quad.arg1 + "\n";
            if (quad.arg1 == "err_bad_index") {
                text_section += "# la $t8, err_bad_index_msg\n";
            }
            text_section += "addi $sp, -4\n";
            text_section += "sw $ra, ($sp)\n";
            text_section += "la $ra, clean_err" + std::to_string(err_labels) + "\n";
            text_section += "jr $t9\n";
            text_section += "clean_err" + std::to_string(err_labels) + ":\n";
            text_section += "la $ra, ($sp)\n";
            text_section += "addi $sp, 4\n";
            text_section += "no_err" + std::to_string(err_labels) + ":\n";
            text_section += "move $t8, $zero\n";
            text_section += "move $t9, $zero\n";
            err_labels++;
        }
        if (op == "to_str") {
            text_section += "addi $sp, -4\n";
            text_section += "sw $a0, ($sp)\n";
            text_section += "addi $sp, -4\n";
            text_section += "sw $a1, ($sp)\n";

            if (ry.reg.starts_with("(")) 
                text_section += "lw $a0, " + ry.reg + "\n";
            else
                text_section += "move $a0, " + ry.reg + "\n";

            if (rz.reg.starts_with("(")) 
                text_section += "lw $a1, " + rz.reg + "\n";
            else
                text_section += "move $a1, " + rz.reg + "\n";

            text_section += "addi $sp, -4\n";
            text_section += "sw $ra, ($sp)\n";
            text_section += "# jal to_string\n";
            text_section += "lw $ra, ($sp)\n";
            text_section += "addi $sp, 4\n";
            text_section += "lw $a1, ($sp)\n";
            text_section += "addi $sp, 4\n";
            text_section += "lw $a0, ($sp)\n";
            text_section += "addi $sp, 4\n";
            text_section += "move " + rx.reg + ", $v0\n"; 
        }
        if (op == "push") {
            text_section += "addi $sp, -4\n";
            text_section += "sw " + ry.reg + ", ($sp)\n";
        }
        if (op == "pop") {
            text_section += "lw " + ry.reg + ", ($sp)\n";
            text_section += "addi $sp, 4\n";
        }
        if (op == "concat") {
            text_section += "addi $sp, -4\n";
            text_section += "sw $a0, ($sp)\n";
            text_section += "addi $sp, -4\n";
            text_section += "sw $a1, ($sp)\n";
            text_section += "move $a0, " + ry.reg + "\n";

            if (rz.reg == "$a0")
                text_section += "lw $a1, 4($sp)\n";
            else
                text_section += "move $a1, " + rz.reg + "\n";

            text_section += "addi $sp, -4\n";
            text_section += "sw $ra, ($sp)\n";
            text_section += "# jal concat_strings\n";
            text_section += "lw $ra, ($sp)\n";
            text_section += "addi $sp, 4\n";
            text_section += "lw $a1, ($sp)\n";
            text_section += "addi $sp, 4\n";
            text_section += "lw $a0, ($sp)\n";
            text_section += "addi $sp, 4\n";
            text_section += "move " + rx.reg + ", $v0\n";
        }
        if (op == "+") text_section += "add " + rx.reg + ", " + ry.reg + ", " + rz.reg + "\n";
        if (op == "-") text_section += "sub " + rx.reg + ", " + ry.reg + ", " + rz.reg + "\n";
        if (op == "*") {
            text_section += "mult " + ry.reg + ", " + rz.reg + "\n";
            text_section += "mflo " + rx.reg + "\n";
        }
        if (op == "/") {
            text_section += "div " + ry.reg + ", " + rz.reg + "\n";
            text_section += "mflo " + rx.reg + "\n";
        }
        if (op == "<") text_section += "slt " + rx.reg + ", " + ry.reg + ", " + rz.reg + "\n";
        if (op == ">") text_section += "sgt " + rx.reg + ", " + ry.reg + ", " + rz.reg + "\n";
        if (op == "<=") text_section += "sle " + rx.reg + ", " + ry.reg + ", " + rz.reg + "\n";
        if (op == ">=") text_section += "sge " + rx.reg + ", " + ry.reg + ", " + rz.reg + "\n";
        if (op == "!=") text_section += "seq " + rx.reg + ", " + ry.reg + ", " + rz.reg + "\n";
        if (op == "==") text_section += "sne " + rx.reg + ", " + ry.reg + ", " + rz.reg + "\n";
        if (op == "&&") text_section += "and " + rx.reg + ", " + ry.reg + ", " + rz.reg + "\n";
        if (op == "||") text_section += "or " + rx.reg + ", " + ry.reg + ", " + rz.reg + "\n";
        if (op == "!") text_section += "not " + rx.reg + ", " + ry.reg + "\n";

        // Clear registers with inmediate values
        for (auto &reg: temporaries) if (std::regex_match(reg, std::regex("[0-9]+"))) reg.clear();
    }
    return text_section + "\n";
}

std::string Mips::generateAssembly() {
    assembly += ".data\n";
    assembly += generateDataSection();
    assembly += ".text\n";
    assembly += generateTextSection();

    return assembly;
}
