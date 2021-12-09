#include "calculator.h"

bool Calculator::EvaluateExpression(std::mt19937& rng, std::shared_ptr<std::vector<Instruction>> expr, std::shared_ptr<std::vector<RollSummary>> roll_summaries, std::shared_ptr<double> t, std::shared_ptr<std::vector<int>> bracket_locations){

	float total = 0;

	// Brackets
	for (int i = 0; i < expr->size(); i++) {

		if (bracket_locations->size() > 0) {
			// Process inside of brackets

			int open_bracket_index = bracket_locations->front();
			int close_bracket_index = 0;
			int bracket_iterator = 0;

			// Increment bracket counter for every open bracket
			// Decrement bracket counter for every close bracket
			// When counter = 0, we have a pair to evaluate
			int bracket_counter = 0;
			for (bracket_iterator = 0; bracket_iterator < bracket_locations->size(); bracket_iterator++) {
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

			if (close_bracket_index - open_bracket_index > 1) {
				// There is an expression to be processed here

				// Isolate the expression inside the brackets for processing
				std::shared_ptr<std::vector<Instruction>> sub_expression = std::make_shared<std::vector<Instruction>>();
				sub_expression->assign(expr->begin() + open_bracket_index + 1, expr->begin() + close_bracket_index);

				// Same for the vector of bracket locations
				std::shared_ptr<std::vector<int>> sub_expression_brackets = std::make_shared<std::vector<int>>();
				sub_expression_brackets->assign(bracket_locations->begin() + 1, bracket_locations->begin() + bracket_iterator);

				// Container to catch the result of the sub-expression
				std::shared_ptr<double> sub_expression_total = std::make_shared<double>(0.0);

				// Offset the sub-expression brackets
				for (int i = 0; i < sub_expression_brackets->size(); i++) {
					sub_expression_brackets->at(i) -= open_bracket_index + 1;
				}

				// Evaluate the sub expression
				if (!EvaluateExpression(rng, sub_expression, roll_summaries, sub_expression_total, sub_expression_brackets))
					return 0;

				// Store the calculated result in the overall expression vector
				expr->at(close_bracket_index) = Instruction(Command::NUM, *sub_expression_total);

				// Remove the bracketed expression from the overall expression
				expr->erase(expr->begin() + open_bracket_index, expr->begin() + close_bracket_index);

				// Remove used brackets from the bracket index list
				bracket_locations->erase(bracket_locations->begin(), bracket_locations->begin() + bracket_iterator + 1);

				// Offset for removed entries in expr
				for (int i = 0; i < bracket_locations->size(); i++) {
					if (bracket_locations->at(i) > open_bracket_index) {
						bracket_locations->at(i) -= (close_bracket_index - open_bracket_index);
					}
				}
			}
			else {
				// An empty set of brackets
			}
		}
	}

	// Process rolls
	for (int i = 0; i < expr->size(); i++) {
		if (expr->at(i).command == Command::DIE) {

			int roll_index = expr->at(i).operand;

			if (roll_summaries->at(roll_index).bracketed_number_of_dice) {
				// We need to fetch the number of dice to be rolled from a previous expression
				if (expr->at(i - 1).command == Command::NUM) {
					roll_summaries->at(roll_index).number_of_dice = expr->at(i - 1).operand;
				}
				else
					return 0;
			}
			if (roll_summaries->at(roll_index).bracketed_number_of_sides) {
				// We need to fetch the number of dice to be rolled from a previous expression
				if (expr->at(i - 1).command == Command::NUM) {
					roll_summaries->at(expr->at(i).operand).number_of_sides = expr->at(i + 1).operand;

				}
				else
					return 0;
			}

			// Roll
			roll_summaries->at(roll_index) = roller.RollDice(rng, roll_summaries->at(roll_index));
			// Replace instruction with totals
			expr->at(i) = Instruction(Command::NUM, roll_summaries->at(roll_index).total);

			if (roll_summaries->at(roll_index).bracketed_number_of_sides) {
				expr->erase(expr->begin() + i + 1);
			}
			if (roll_summaries->at(roll_index).bracketed_number_of_dice) {
				expr->erase(expr->begin() + i - 1);
			}
		}
	}

	// Indices
	for (int i = 1; i < expr->size(); i++) {
		if (expr->at(i).command == Command::IND) {
			// POWAH
			if (expr->at(i - 1).command != Command::NUM || expr->at(i + 1).command != Command::NUM)
				return 0;// Can't take the power of a command. Brackets are already evaluated by this point and so cannot occur here

			// Calculate power
			float total = powf(expr->at(i - 1).operand, expr->at(i + 1).operand);

			// Store back in location
			expr->at(i + 1) = Instruction(Command::NUM, total);
			expr->erase(expr->begin() + i - 1, expr->begin() + i + 1);
		}
	}

	// Multiplication / Division
	for (int i = 1; i < expr->size(); i++) {
		if (expr->at(i).command == Command::DIV) {
			bool isnum2negative = false;
			if (expr->at(i + 1).command == Command::SUB) {
				isnum2negative = true;
				expr->erase(expr->begin() + i + 1);
			}
			else if (expr->at(i + 1).command == Command::ADD) {
				expr->erase(expr->begin() + i + 1);
			}

			if (expr->at(i - 1).command != Command::NUM || expr->at(i + 1).command != Command::NUM || (i + 1 >= expr->size()))
				return 0; // Need two numbers

			// Fetch numbers
			float num1 = expr->at(i - 1).operand;
			float num2 = expr->at(i + 1).operand;

			if (isnum2negative)
				num2 = -num2;

			if (num2 == 0.0) {
				// Divide by zero
				return 0;
			}

			// Calculate
			float total = num1 / num2;

			// Store back in correct location
			expr->at(i + 1) = Instruction(Command::NUM, total);
			expr->erase(expr->begin() + i - 1, expr->begin() + i + 1);
			i--; // Fix iterator
		}
		else if (expr->at(i).command == Command::MUL) {
			// MULTIPLY

			// Check if the second number is using a negative operator
			bool isnum2negative = false;
			if (expr->at(i + 1).command == Command::SUB) {
				isnum2negative = true;
				expr->erase(expr->begin() + i + 1);
			}
			else if (expr->at(i + 1).command == Command::ADD) {
				expr->erase(expr->begin() + i + 1);
			}

			if (expr->at(i - 1).command != Command::NUM || expr->at(i + 1).command != Command::NUM || (i + 1 >= expr->size()))
				return 0; // Need two numbers

			// Fetch numbers
			float num1 = expr->at(i - 1).operand;
			float num2 = expr->at(i +  1).operand;

			if (isnum2negative)
				num2 = -num2;

			// Calculate
			float total = num1 * num2;

			// Store back in correct location
			expr->at(i + 1) = Instruction(Command::NUM, total);
			expr->erase(expr->begin() + i - 1, expr->begin() + i + 1);
			i--;// Fix iterator
		}
	}

	// Addition / Subtraction
	// Iterate through to eliminate double negatives etc. 
	for (int i = 0; i < expr->size(); i++) {
		if (expr->at(i).command != Command::NUM) {
			// This entry is not a digit
			if (i + 1 >= expr->size())
				return 0;

			if (expr->at(i).command != Command::NUM) {
				// The next entry is not a digit either - evaluate the outcome of these 
				if (expr->at(i).command == Command::ADD && expr->at(i + 1).command == Command::ADD) {
					expr->erase(expr->begin() + i + 1);
					i--;
				}
				else if (expr->at(i).command == Command::SUB && expr->at(i + 1).command == Command::SUB) {
					expr->erase(expr->begin() + i + 1);
					expr->at(i) = Instruction(Command::ADD);
					i--;
				}
				else if ((expr->at(i).command == Command::ADD && expr->at(i + 1).command == Command::SUB) || (expr->at(i).command == Command::SUB && expr->at(i + 1).command == Command::ADD)) {
					expr->erase(expr->begin() + i + 1);
					expr->at(i) = Instruction(Command::SUB);
					i--;
				}
			}
		}
	}

	// Evaluate by adding or subtracting each number element with total
	for (int i = 0; i < expr->size(); i++) {
		float num = 0;
		if (expr->at(i).command == Command::SUB) {
			// Subtract the following number from the total
			if (i + 1 >= expr->size())
				return 0;
			num = -expr->at(i + 1).operand;
			expr->erase(expr->begin(), expr->begin() + 2);
		}
		else if (expr->at(i).command == Command::ADD) {
			// Add the following number to the total
			if (i + 1 >= expr->size())
				return 0;
			num = expr->at(i + 1).operand;
			expr->erase(expr->begin(), expr->begin() + 2);
		}
		else {
			// Number on its own - this is usually the number at the start of the expression, so we add it to the total
			num = expr->at(i).operand;
			expr->erase(expr->begin());
		}
		i--;// Fix iterator
		total = total + num;
	}

	*t = total;

	return 1;

}