#include <cassert>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "parser_noh.hpp"
#include "eval_noh.hpp"

int main(int argc, const char* argv[])
{
	std::string input{}, s;
	assert(argc >= 2);
	std::string fName(argv[1]);
	std::ifstream fIn(fName);
	if(not fIn.is_open())
	{
		std::cerr << "could not open the file: '" << fName << "'" << std::endl;
		return EXIT_FAILURE;
	}
	if(std::filesystem::path(fName).extension() != ".noh")
	{
		std::cerr << "invalid extension: '" << fName << "'" << std::endl;
		return EXIT_FAILURE;
	}

	while(std::getline(fIn, s)) { input += s; input += '\n'; }
	fIn.close();

	auto it = std::begin(input);
	Noh::parser::Calc<std::string::iterator, qi::standard_wide::space_type> calc;
	Noh::ast::ModuleAst* res = nullptr;
	bool success = qi::phrase_parse(
		it,
		std::end(input),
		calc,
		qi::standard_wide::space,
		res
	);

	if(success and it == std::end(input))
	{
		Noh::eval::AstEval asteval(res);
	}
	else { std::cout << "parse failed" << std::endl; }

	return EXIT_SUCCESS;
}