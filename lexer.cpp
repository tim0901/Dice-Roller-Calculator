#include "lexer.h"

std::tuple<std::string, int>  Lexer::LoadWord(std::string str, int i, int offset) {
	std::string tempword = "";
	while ((i + offset) < str.length()) {
		if (str[i + offset] == ' ') {
			// Skip spaces
			offset++;
		}
		else if (isalpha(str[i + offset]) || str[i+offset] == '<' || str[i+offset] == '>') {
			// The next character is also relevant
			tempword += str[i + offset];
			offset++;
		}
		else {
			break;
		}
	}
	return { tempword, offset };
}

std::tuple<int, int, int> FindBracketedExpressionAfterPoint(std::shared_ptr<std::vector<Instruction>> expr, std::shared_ptr<std::vector<int>> bracket_locations, int starting_index) {
	int open_bracket_index = 0, close_bracket_index = 0;

	// Find first LBR instruction after the starting index. Store its location in the full expression as well as in the bracket locations list
	int bracket_iterator = 0;
	for (int i = 0; i < bracket_locations->size(); i++) {
		if (bracket_locations->at(i) > starting_index && expr->at(bracket_locations->at(i)).command == Command::LBR) {
			open_bracket_index = bracket_locations->at(i);
			bracket_iterator = i;
			break;
		}

	}

	// Increment bracket counter for every open bracket
	// Decrement bracket counter for every close bracket
	// When counter = 0, we have a pair to evaluate

	int bracket_counter = 0;
	for (bracket_iterator; bracket_iterator < bracket_locations->size(); bracket_iterator++) {
		if (bracket_locations->at(bracket_iterator) > starting_index) {
			if (expr->at(bracket_locations->at(bracket_iterator)).command == Command::LBR) {
				bracket_counter++;
			}
			else {
				bracket_counter--;
			}
			if (bracket_counter == 0) {
				// we have a pair
				close_bracket_index = bracket_locations->at(bracket_iterator);
				break;
			}
		}
	}

	return { open_bracket_index,close_bracket_index, bracket_iterator };
}

std::tuple<std::string, int> Lexer::LoadNumber(std::string str, int i, int offset) {
	std::string tempword = "";
	while (i + offset < str.length()) {
		// Keep lexing to find the whole number - for ints or floats
		if (str[i + offset] == ' ') {
			offset++;
		}
		else if (isdigit(str[i + offset])) {
			// The next digit is also a number
			tempword += str[i + offset];
			offset++;
		}
		else if (str[i + offset] == '.') {
			// Decimal point - it's a float
			if (tempword == "") {
				tempword += '0';
			}
			tempword += ".";
			offset++;
		}
		else {
			break;
		}
	}
	return { tempword, offset };
}

bool Lexer::Lex(std::mt19937& rng, std::string str, std::shared_ptr<std::string> output, std::shared_ptr<double> t) {

	std::shared_ptr<std::vector<Instruction>> instructions = std::make_shared<std::vector<Instruction>>();

	// Flatten input
	for (int i = 0; i < str.length(); i++) {
		str[i] = tolower(str[i]);
	}

	// Use a regex to process and remove advantage/disadvantage from the input string
	std::shared_ptr<int> advantage = std::make_shared<int>();

	std::regex advantage_regex("adv");
	std::regex disadvantage_regex("dis");
	if (std::regex_search(str, advantage_regex)) {
		str = std::regex_replace(str, advantage_regex, "");
		*advantage = 1;
	}
	if (std::regex_search(str, disadvantage_regex)) {
		str = std::regex_replace(str, disadvantage_regex, "");
		if (*advantage == 1) {
			*advantage = 0;
		}
		else {
			*advantage = 2;
		}
	}

	// Useful locations to cache now
	std::shared_ptr<std::vector<int>> bracket_locations = std::make_shared<std::vector<int>>();
	std::shared_ptr<std::vector<int>> roll_locations = std::make_shared<std::vector<int>>();
	std::shared_ptr<std::vector<RollSummary>> roll_summaries = std::make_unique<std::vector<RollSummary>>();

	bool result = Lex_Expression(str, instructions, roll_summaries, bracket_locations, roll_locations, advantage);

	if (!result || bracket_locations->size() % 2 != 0)
		return 0;

	// Evaluate expression
	Calculator calc;
	std::shared_ptr<std::vector<Instruction>> instructions_copy = std::make_shared<std::vector<Instruction>>(*instructions); // For writing out at the end
	std::shared_ptr<std::vector<int>> brackets_copy = std::make_shared<std::vector<int>>(*bracket_locations); // For writing out at the end
	result = calc.EvaluateExpression(rng, instructions, roll_summaries, t, bracket_locations);

	if (result) {
		*output = GenerateWrittenExpression(instructions_copy, roll_summaries, brackets_copy);
	}

	return result;
}
bool Lexer::Lex_Expression(std::string expr, std::shared_ptr<std::vector<Instruction>> instructions, std::shared_ptr<std::vector<RollSummary>> roll_summaries, std::shared_ptr<std::vector<int>> bracket_locations, std::shared_ptr<std::vector<int>> roll_locations, std::shared_ptr<int> advantage) {

	for (int i = 0; i < expr.length(); i++) {
		int offset = 0;
		std::string tempword = "";

		if (isalpha(expr[i])) {

			// Find whole word
			tie(tempword, offset) = LoadWord(expr, i, offset);

			// Process die rolls now
			if (tempword == "d") {
				int num1, num2 = 0;

				// Container to store all information about this roll
				RollSummary summary;
				roll_summaries->push_back(summary);
				int roll_summary_index = roll_summaries->size() - 1;

				// Is there a number before and after this one
				// Check if the previous word was a number - if so use it as num1
				if (instructions->size() > 0 && instructions->at(instructions->size() - 1).command == Command::NUM) {
					num1 = (int)instructions->at(instructions->size() - 1).operand;

					// Remove the previous entry to prevent accidental re-use
					instructions->pop_back();
				}
				else if (instructions->size() > 0 && instructions->at(instructions->size() - 1).command == Command::RBR) {
					// We have a bracketed expression here that needs to be evaluated first. 
					// Set flag to true and this value will be fetched later
					roll_summaries->at(roll_summary_index).bracketed_number_of_dice = true;
					num1 = 0;
				}
				else {
					// Otherwise assume 1
					num1 = 1;
				}

				// Push the instruction for the roll to the instructions vector
				// The details of the roll are stored inside roll_summaries
				instructions->push_back(Instruction(Command::DIE, roll_summary_index));

				// Store the location of the roll for use when writing the expression at the end
				roll_locations->push_back(instructions->size() - 1);


				// Check for a valid expression to use as num2 - the number of sides of the dice to be rolled
				
				// Skip past spaces
				while (expr[i + offset] == ' ') {
					offset++;
				}
				
				tempword = "";
				if (isdigit(expr[i + offset])) {
					// If it's a number - use that
					tie(tempword, offset) = LoadNumber(expr, i, offset);
					num2 = atoi(tempword.c_str());
					if (num2 == 0) {
						return 0;
					}

				}
				else if (expr[i + offset] == '(') {
					// We have a bracketed expression here that needs to be evaluated first. 
					// Set flag to true and this value will be fetched later
					roll_summaries->at(roll_summary_index).bracketed_number_of_sides = true;
					
					// And now we parse the sub-expression

					// Step 1: find the matching bracket;
					int bracket_counter = 0;
					int bracket_location = 0;
					for (bracket_location = 0; i + bracket_location + offset < expr.size(); bracket_location++) {
						if (expr[i + bracket_location + offset] == '(') {
							bracket_counter++;
						}
						else if (expr[i + bracket_location + offset] == ')') {
							bracket_counter--;
						}
						if (bracket_counter == 0) {
							break;
						}
					}
					if (bracket_counter != 0) {
						return 0;
					}

					// Step 2: extract sub-expression
					// First bracket is at i + offset, second is at i + offset + bracket_location
					std::string sub_expression = expr.substr(i + offset + 1, bracket_location - 1);

					// Step 3: process sub-expression

					instructions->push_back(Instruction(Command::LBR));
					bracket_locations->push_back(instructions->size() - 1);
					bool result = Lex_Expression(sub_expression, instructions, roll_summaries, bracket_locations, roll_locations, advantage);
					instructions->push_back(Instruction(Command::RBR));
					bracket_locations->push_back(instructions->size() - 1);

					// Step 4: fix iterator
					offset += bracket_location + 1;

					num2 = 0;

				}
				else {
					// Invalid input. 
					return 0;
				}

				// Look for modifiers
				tempword = "";
				int modifier_offset = offset;
				tie(tempword, modifier_offset) = LoadWord(expr, i, modifier_offset);
				int op = 0, mod = 0;

				if (tempword != "") {
					std::string modifier = "";
					tie(modifier, modifier_offset) = LoadNumber(expr, i, modifier_offset);
					if (modifier == "")
						modifier = "1";

					if (tempword == "kh" || tempword == "k") {
						// Keep highest = drop lowest
						op = 3;
						mod = (atoi(modifier.c_str()));
						offset = modifier_offset;
					}
					else if (tempword == "kl") {
						// Keep lowest = drop highest
						op = 4;
						mod = atoi(modifier.c_str());
						offset = modifier_offset;
					}
					else if (tempword == "dh") {
						// Drop highest
						op = 4;
						mod = (num1 - atoi(modifier.c_str()));
						offset = modifier_offset;
					}
					else if (tempword == "dl" || tempword == "d") {
						// Drop lowest
						op = 3;
						mod = (num1 - atoi(modifier.c_str()));
						offset = modifier_offset;
					}
					else if (tempword == "min" || tempword == "mi") {
						// Set a minimum value for a roll
						op = 5;
						mod = atoi(modifier.c_str());
						offset = modifier_offset;
					}
					else if (tempword == "max" || tempword == "ma") {
						// Set a minimum value for a roll
						op = 6;
						mod = atoi(modifier.c_str());
						offset = modifier_offset;
					}
					else if (tempword == "r" || tempword == "ro") {
						// Reroll values eg 1s
						op = 7;
						mod = atoi(modifier.c_str());
						offset = modifier_offset;
					}
					else if (tempword == "r<") {
						// Reroll values less than 
						op = 8;
						mod = atoi(modifier.c_str());
						offset = modifier_offset;
					}
					else if (tempword == "r>") {
						// Reroll values greater than
						op = 9;
						mod = atoi(modifier.c_str());
						offset = modifier_offset;
					}
				}

				// Ignore "adv" or "dis" when modifiers are used or when rolling more than one die
				if (op != 0 || num1 != 1) {
					advantage = 0;
					// Retroactively apply to previously found rolls
					for (int j = 0; j < roll_summaries->size(); j++) {
						if (roll_summaries->at(j).op == 1 || roll_summaries->at(j).op == 2) {
							roll_summaries->at(j).op = 0;
							roll_summaries->at(j).mod = 0;
						}
					}
				}
				else {
					op = *advantage;
					if (*advantage > 0) {
						mod = 1;
					}
				}

				// Store the instruction

				// Store roll to be processed later
				roll_summaries->at(roll_summary_index).number_of_dice = num1;
				roll_summaries->at(roll_summary_index).number_of_sides = num2;
				roll_summaries->at(roll_summary_index).op = op;
				roll_summaries->at(roll_summary_index).mod = mod;

			}

			// Fix iterator
			i += (offset - 1);
		}
		else if (isdigit(expr[i]) || expr[i] == '.') {
			// Found a number
			tie(tempword, offset) = LoadNumber(expr, i, offset);
			i += (offset - 1);

			// Store the number
			instructions->push_back(Instruction(Command::NUM, atof(tempword.c_str())));
		}
		else if (expr[i] == '+') {
			// Addition
			instructions->push_back(Instruction(Command::ADD));
		}
		else if (expr[i] == '-') {
			// Subtraction
			instructions->push_back(Instruction(Command::SUB));
		}
		else if (expr[i] == '*') {
			// Multiplication
			instructions->push_back(Instruction(Command::MUL));
		}
		else if (expr[i] == '/') {
			// Division
			instructions->push_back(Instruction(Command::DIV));
		}
		else if (expr[i] == '(') {
			// LBR

			// Check for implicit multiplication
			if (instructions->size() > 0) {
				Command previous_command = instructions->at(instructions->size() - 1).command;
				if (previous_command == Command::NUM || previous_command == Command::RBR)
					instructions->push_back(Instruction(Command::MUL));
			}

			instructions->push_back(Instruction(Command::LBR));
			bracket_locations->push_back(instructions->size() - 1);
		}
		else if (expr[i] == ')') {
			// RBR
			instructions->push_back(Instruction(Command::RBR));
			bracket_locations->push_back(instructions->size() - 1);
		}
		else if (expr[i] == '^') {
			// Indice
			instructions->push_back(Instruction(Command::IND));
		}
		else {
			// Skip any other characters
		}
	}
	return 1;
}

std::string Lexer::GenerateWrittenExpression(std::shared_ptr<std::vector<Instruction>> instructions, std::shared_ptr<std::vector<RollSummary>> roll_summaries, std::shared_ptr<std::vector<int>> bracket_locations) {

	std::ostringstream stream;

	for (int i = 0; i < instructions->size(); i++) {
		if (instructions->at(i).command == Command::DIE) {
			// A wild roll appeared

			RollSummary summary = roll_summaries->at(instructions->at(i).operand);

			// Add an extra space when required
			if (i != 0 && instructions->at(i - 1).command != Command::LBR && !summary.bracketed_number_of_dice) {
				stream << " ";
			}

			// Standard << operator output is not possible here as the sub-expression must be printed mid-roll
			if (summary.bracketed_number_of_sides) {

				if (!summary.bracketed_number_of_dice) {
					stream << summary.number_of_rolls;
				}

				stream << "d";

				// Obtain the bracketed expression following this instruction
				int open_bracket_index = 0, close_bracket_index = 0, bracket_iterator = 0;
				std::tie(open_bracket_index, close_bracket_index, bracket_iterator) = FindBracketedExpressionAfterPoint(instructions, bracket_locations, i);

				// Process sub-expression
				int sub_expression_size = close_bracket_index - open_bracket_index;
				if (sub_expression_size > 1) {
					// Isolate the expression inside the brackets for processing
					std::shared_ptr<std::vector<Instruction>> sub_expression = std::make_shared<std::vector<Instruction>>();
					sub_expression->assign(instructions->begin() + open_bracket_index + 1, instructions->begin() + close_bracket_index);

					// Same for the vector of bracket locations
					std::shared_ptr<std::vector<int>> sub_expression_brackets = std::make_shared<std::vector<int>>();
					sub_expression_brackets->assign(bracket_locations->begin() + 1, bracket_locations->begin() + bracket_iterator);

					// Offset the sub-expression brackets
					for (int i = 0; i < sub_expression_brackets->size(); i++) {
						sub_expression_brackets->at(i) -= open_bracket_index + 1;
					}

					// Evaluate sub-expression
					stream << "(" << GenerateWrittenExpression(sub_expression, roll_summaries, sub_expression_brackets) << ")";
				}

				stream << summary;

				// Fix iterator
				i += sub_expression_size + 1;

			}
			else {
				stream << summary;
			}

		}
		else {
			stream << instructions->at(i);
		}
	}


	std::string output = stream.str();
	// Remove extra spaces
	output = std::regex_replace(output, std::regex("^ +| +$|( ) +"), "$1");

	return output;
}