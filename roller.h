#pragma once

#include <vector>
#include <random>
#include <memory>
#include <string>
#include <iostream>

struct RollSummary {
	bool bracketed_number_of_dice = false;
	bool bracketed_number_of_sides = false;
	int number_of_dice = 0;
	int number_of_rolls = 0;
	int number_of_sides = 0;
	int op = 0;
	int mod = 0;
	std::vector<int> accepted_rolls;
	std::vector<int> rejected_roll_indices;
	int total = 0;
};


inline std::ostream& operator << (std::ostream& os, const RollSummary& s) {

	// Print roll expression eg 2d20kh1
	if (s.bracketed_number_of_sides) {

	}
	else if (s.bracketed_number_of_dice) {
		os << "d" << s.number_of_sides;
	}
	else {
		os << s.number_of_rolls << "d" << s.number_of_sides;
	}

	os << (s.op == 3 ? "kh" : s.op == 4 ? "kl" : s.op == 5 ? "min" : s.op == 6 ? "max" : s.op == 7 ? "r" : s.op == 8 ? "r<" : s.op == 9 ? "r>" : "")
		<< (s.mod == 0 ? "" : (5 <= s.op <= 9) ? std::to_string(s.mod) : std::to_string(s.number_of_rolls - s.mod)) << " (";

	// Print accepted and rejected rolls in brackets
	for (int i = 0; i < s.accepted_rolls.size(); i++) {
		bool roll_printed = false;
		int r = s.accepted_rolls.at(i);
		for (int j = 0; j < s.rejected_roll_indices.size(); j++) {

			if (i == s.rejected_roll_indices.at(j)) {

				// Match - this roll was marked as rejected
				if ((r == s.number_of_sides || r == 1) && ((r < s.mod && s.op == 5) || (r > s.mod && s.op == 6)))
					os << "~~**" << r << "**~~ -> " << s.mod << ", ";
				else if ((r < s.mod && s.op == 5) || (r > s.mod && s.op == 6))
					os << "~~" << r << "~~ -> " << s.mod << ", ";
				else if ((r == s.number_of_sides || r == 1) && ((r == s.mod && s.op == 7) || (r < s.mod && s.op == 8) || (r > s.mod && s.op == 9))) {
					os << "~~**" << r << "**~~ -> ";
					r = s.accepted_rolls.at(i++ + 1);
					if (r == s.number_of_sides || r == 1)
						os << "**" << r << "**, ";
					else
						os << r << ", ";
				}
				else if ((r == s.mod && s.op == 7) || (r < s.mod && s.op == 8) || (r > s.mod && s.op == 9)) {
					os << "~~" << r << "~~ -> ";
					r = s.accepted_rolls.at(i++ + 1);
					if (r == s.number_of_sides || r == 1)
						os << "**" << r << "**, ";
					else
						os << r << ", ";
				}
				else if (r == s.number_of_sides || r == 1)
					os << "~~**" << r << "**~~, ";
				else
					os << "~~" << r << "~~, ";
				roll_printed = true; // Prevent double outputs
				break;
			}

		}
		if (!roll_printed) {
			// This roll was accepted
			if (r == s.number_of_sides || r == 1)
				os << "**" << r << "**, ";
			else
				os << r << ", ";
		}
	}
	os.seekp(-2, std::ios_base::end); // Reverse two characters to overwrite last comma
	os << ")";

	return os;
}

class Roller {
public:
	Roller() {}
	~Roller() {}

	RollSummary RollDice(std::mt19937& rng, int n, int sides, int o = 0, int mod = 0);

	RollSummary RollDice(std::mt19937& rng, RollSummary);

	void InsertionSortSmallToBig(std::shared_ptr<std::vector<int>> numbers);
	void InsertionSortBigToSmall(std::shared_ptr<std::vector<int>> numbers);
};

