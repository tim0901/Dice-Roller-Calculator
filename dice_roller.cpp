// DiceRoller.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>


#include <regex>
#include <vector>
#include <random>
#include <memory>
#include "lexer.h"

// Expose the following functions
extern "C" {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	__declspec(dllexport) Lexer* Lexer_new() { return new Lexer(); }
	__declspec(dllexport) const char* Lexer_Lex(Lexer* Lex, const char* input, char* output_buffer, unsigned int output_buffer_size, double* total, double override_rng = 0);
#else
	Lexer* Lexer_new() { return new Lexer(); }
	const char* Lexer_Lex(Lexer* Lex, const char* input, char* output_buffer, unsigned int output_buffer_size, double* total, double override_rng = 0);
#endif
}

const char* Lexer_Lex(Lexer* Lex, const char* input, char* output_buffer, unsigned int output_buffer_size, double* total, double override_rng) {
	// Convert to c++ types
	std::shared_ptr<std::string> shared_output = std::make_shared<std::string>("");
	std::shared_ptr<double> shared_total = std::make_shared<double>(0);
	bool result = false;

	if (override_rng != 0) {
		// Override rng with fixed seed for testing situations
		std::mt19937 rng(override_rng);
		result = Lex->Lex(rng, input, shared_output, shared_total);
	}
	else {
		std::random_device r;
		std::mt19937 rng(r());
		result = Lex->Lex(rng, input, shared_output, shared_total);
	}

	// Return results
	if (!result) {
		shared_output = std::make_shared<std::string>("Invalid expression");
	}

	if (shared_output->size() > output_buffer_size) {
		std::cerr << "Buffer too small" << std::endl;
		return "";
	}
	shared_output->copy(output_buffer, output_buffer_size);
	*total = *shared_total;
	return output_buffer;
}