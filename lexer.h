#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <regex>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include "calculator.h"

class Lexer
{
public:
	Lexer() {}
	~Lexer() {}

	std::tuple<std::string, int> LoadWord(std::string str, int i, int offset);
	std::tuple<std::string, int> LoadNumber(std::string str, int i, int offset);

	bool Lex(std::mt19937& rng, std::string str, std::shared_ptr<std::string> output, std::shared_ptr<double> t);

private:
	bool Lex_Expression(std::string sub_string, std::shared_ptr<std::vector<Instruction>> instructions, std::shared_ptr<std::vector<RollSummary>> roll_summaries, std::shared_ptr<std::vector<int>> bracket_locations, std::shared_ptr<std::vector<int>> roll_locations, std::shared_ptr<int> advantage);
	std::string GenerateWrittenExpression(std::shared_ptr<std::vector<Instruction>> instructions, std::shared_ptr<std::vector<RollSummary>> roll_summaries, std::shared_ptr<std::vector<int>> bracket_locations);
};