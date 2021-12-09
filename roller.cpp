#include "roller.h"

RollSummary Roller::RollDice(std::mt19937& rng, int n, int sides, int o, int mod) {

	RollSummary summary;
	summary.number_of_dice = n;
	summary.number_of_sides = sides;
	summary.op = o;
	summary.mod = mod;

	return RollDice(rng, summary);
}

RollSummary Roller::RollDice(std::mt19937& rng, RollSummary summary) {

	int nRolls = summary.number_of_dice;
	int op = summary.op;
	int modifier = summary.mod;

	// Create die
	std::uniform_int_distribution<int> die(1, summary.number_of_sides);

	std::shared_ptr<std::vector<int>> rolled_dice = std::make_shared<std::vector<int>>();

	// Convert adv/dis to its dl/kh equivalent
	if (op == 1) {
		// ADVANTAGE
		nRolls = 2;
		op = 3;
		modifier = 1;
	}
	else if (op == 2) {
		// Disadvantage
		nRolls = 2;
		op = 4;
		modifier = 1;
	}

	// Roll dice
	for (int i = 0; i < nRolls; i++) {
		rolled_dice->push_back(die(rng));
	}

	switch (op) {
	case 3: {
		// Drop lowest n

		// Get a sorted list of the dice
		std::shared_ptr<std::vector<int>> sorted_dice = std::make_shared<std::vector<int>>(rolled_dice->size());
		std::copy(rolled_dice->begin(), rolled_dice->end(), sorted_dice->begin());
		InsertionSortBigToSmall(sorted_dice);

		// Delete all but the n smallest rolls
		sorted_dice->erase(sorted_dice->begin(), sorted_dice->begin() + modifier);

		for (int i = 0; i < rolled_dice->size(); i++) {
			for (int j = 0; j < sorted_dice->size(); j++) {
				if (rolled_dice->at(i) == sorted_dice->at(j)) {
					// Match

					// Store the index of the rejected roll
					summary.rejected_roll_indices.push_back(i);

					// Remove this roll from the sorted list - we don't want double hits
					sorted_dice->erase(sorted_dice->begin() + j);

					// Fix the total
					summary.total -= rolled_dice->at(i);

					// Escape this loop to avoid double hits
					break;
				}
			}
			summary.accepted_rolls.push_back(rolled_dice->at(i));
			summary.total += rolled_dice->at(i);
		}
		summary.number_of_rolls = summary.number_of_dice;
		break;
	}
	case 4: {
		// Drop highest n

		// Get a sorted list of the dice
		std::shared_ptr<std::vector<int>> sorted_dice = std::make_shared<std::vector<int>>(rolled_dice->size());
		std::copy(rolled_dice->begin(), rolled_dice->end(), sorted_dice->begin());
		InsertionSortSmallToBig(sorted_dice);

		// Delete all but the n smallest rolls
		sorted_dice->erase(sorted_dice->begin(), sorted_dice->begin() + modifier);

		for (int i = 0; i < rolled_dice->size(); i++) {
			for (int j = 0; j < sorted_dice->size(); j++) {
				if (rolled_dice->at(i) == sorted_dice->at(j)) {
					// Match

					// Store the index of the rejected roll
					summary.rejected_roll_indices.push_back(i);

					// Remove this roll from the sorted list - we don't want double hits
					sorted_dice->erase(sorted_dice->begin() + j);

					// Fix the total
					summary.total -= rolled_dice->at(i);

					// Escape this loop to avoid double hits
					break;
				}
			}
			summary.accepted_rolls.push_back(rolled_dice->at(i));
			summary.total += rolled_dice->at(i);
		}
		summary.number_of_rolls = summary.number_of_dice;
		break;
	}
	case 5: {
		// Min
		for (int i = 0; i < nRolls; i++) {
			if (rolled_dice->at(i) < modifier) {
				// Store the index of the rejected roll
				summary.accepted_rolls.push_back(rolled_dice->at(i));
				summary.rejected_roll_indices.push_back(i);
				// Add the minimum value to the total
				summary.total += modifier;
			}
			else {
				// Add the accepted rolls to the total
				summary.accepted_rolls.push_back(rolled_dice->at(i));
				summary.total += rolled_dice->at(i);
			}
		}
		break;
	}
	case 6: {
		// Max
		for (int i = 0; i < nRolls; i++) {
			if (rolled_dice->at(i) > modifier) {
				// Store the index of the rejected roll
				summary.accepted_rolls.push_back(rolled_dice->at(i));
				summary.rejected_roll_indices.push_back(i);
				// Add the maximum value to the total
				summary.total += modifier;
			}
			else {
				// Add the accepted rolls to the total
				summary.accepted_rolls.push_back(rolled_dice->at(i));
				summary.total += rolled_dice->at(i);
			}
		}
		break;
	}
	case 7:
		// Reroll values
		for (int i = 0; i < nRolls; i++) {
			if (rolled_dice->at(i) == modifier) {
				// Reroll
				summary.accepted_rolls.push_back(rolled_dice->at(i));
				summary.rejected_roll_indices.push_back(summary.accepted_rolls.size() - 1);
				rolled_dice->push_back(die(rng));
				summary.accepted_rolls.push_back(rolled_dice->back());
				summary.total += rolled_dice->back();
			}
			else {
				summary.accepted_rolls.push_back(rolled_dice->at(i));
				summary.total += rolled_dice->at(i);
			}
		}
		break;
	case 8: {
		// Reroll values less than
		for (int i = 0; i < nRolls; i++) {
			if (rolled_dice->at(i) < modifier) {
				// Reroll
				summary.accepted_rolls.push_back(rolled_dice->at(i));
				summary.rejected_roll_indices.push_back(summary.accepted_rolls.size() - 1);
				rolled_dice->push_back(die(rng));
				summary.accepted_rolls.push_back(rolled_dice->back());
				summary.total += rolled_dice->back();
			}
			else {
				summary.accepted_rolls.push_back(rolled_dice->at(i));
				summary.total += rolled_dice->at(i);
			}
		}
		break;
	}
	case 9: {
		// Reroll values greater than
		for (int i = 0; i < nRolls; i++) {
			if (rolled_dice->at(i) > modifier) {
				// Reroll
				summary.accepted_rolls.push_back(rolled_dice->at(i));
				summary.rejected_roll_indices.push_back(summary.accepted_rolls.size() - 1);
				rolled_dice->push_back(die(rng));
				summary.accepted_rolls.push_back(rolled_dice->back());
				summary.total += rolled_dice->back();
			}
			else {
				summary.accepted_rolls.push_back(rolled_dice->at(i));
				summary.total += rolled_dice->at(i);
			}
		}
		break;
	}
	default: {
		summary.accepted_rolls.swap(*rolled_dice);
		for (int i = 0; i < summary.accepted_rolls.size(); i++)
			summary.total += summary.accepted_rolls.at(i);
		break;
	}
	}

	summary.number_of_rolls = nRolls;
	summary.mod = modifier;
	summary.op = op;
	
	return summary;
}
void Roller::InsertionSortSmallToBig(std::shared_ptr<std::vector<int>> numbers) {
	// Use an insertion sort to sort integers
	// Smaller index = smaller number
	if (numbers->size() <= 1) {
		return;
	}
	for (int i = 1; i < numbers->size(); i++) {
		for (int j = i; j > 0; j--) {
			if (numbers->at(j) < numbers->at(j - 1)) {
				std::swap(numbers->at(j), numbers->at(j - 1));
			}
		}
	}
}

void Roller::InsertionSortBigToSmall(std::shared_ptr<std::vector<int>> numbers) {
	// Use an insertion sort to sort integers
	// Smaller index = bigget number
	if (numbers->size() <= 1) {
		return;
	}
	for (int i = 1; i < numbers->size(); i++) {
		for (int j = i; j > 0; j--) {
			if (numbers->at(j) > numbers->at(j - 1)) {
				std::swap(numbers->at(j), numbers->at(j - 1));
			}
		}
	}
}