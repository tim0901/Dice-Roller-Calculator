#pragma once

#include <math.h>
#include <vector>
#include <memory>
#include <iostream>
#include "roller.h"

enum class Command {
	ADD, SUB, MUL, DIV, IND, LBR, RBR, NUM, DIE
};

struct Instruction {
public:
	Instruction(Command c, float o = 0.0) { command = c; operand = o; }
	Command command;
	float operand;
};

inline std::ostream& operator << (std::ostream& os, const Instruction& i) {
	switch (i.command) {
	case Command::ADD: {
		os << " + ";
		break;
	}
	case Command::SUB: {
		os << " - ";
		break;
	}
	case Command::MUL: {
		os << " * ";
		break;
	}
	case Command::DIV: {
		os << " / ";
		break;
	}
	case Command::IND: {
		os << "^";
		break;
	}
	case Command::LBR: {
		os << "(";
		break;
	}
	case Command::RBR: {
		os << ")";
		break;
	}
	case Command::NUM: {
		os << i.operand;
		break;
	}
	case Command::DIE: {
		break;
	}
	}

	return os;
}

class Calculator
{
public:
	Calculator() {};
	~Calculator() {};

	bool EvaluateExpression(std::mt19937& rng, std::shared_ptr<std::vector<Instruction>> expr, std::shared_ptr<std::vector<RollSummary>> roll_summaries, std::shared_ptr<double> t, std::shared_ptr<std::vector<int>> bracket_locations);

private:
	Roller roller;
};
