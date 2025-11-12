#pragma once

#include "SemanticChecker.h"
#include "IRGenerator.h"

void test_stream(const std::string &stream, CompiScript::SemanticChecker *checker);
std::string test_ir_gen(const std::string &stream);
std::string test_mips_gen(const std::string &stream);
