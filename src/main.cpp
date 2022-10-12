#include <cassert>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <boost/program_options.hpp>

#include "parser_noh.hpp"
#include "eval_noh.hpp"
#include "version_noh.hpp"

int main(int argc, const char* argv[])
{
	namespace po = boost::program_options;
	po::options_description desc(
		"Usage: ./NohEval [options]\n"
		"Options"
	);
	
	desc.add_options()
		("help,h", "Show this help message.")
		("version,v", "Show version information.")
		("input,i", po::value<std::string>(), "Input file path.");
	po::positional_options_description pos_desc;
	pos_desc.add("input", 1);
	
	po::variables_map vm;

	try
	{
		po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_desc).run(), vm);
		po::notify(vm);
	}
	catch (const po::error& e)
	{
		std::cout << e.what() << std::endl;
		std::cout << desc << std::endl;
		return EXIT_FAILURE;
	}

	if (vm.count("help"))
	{
		std::cout << desc << std::endl;
	}
	if (vm.count("version"))
	{
		std::cout << "Noh version " << NOH_VERSION_STRING << std::endl;
	}
	if (vm.count("input"))
	{
		try
		{
			const std::string fName(vm["input"].as<std::string>());
			std::string input{}, s;

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

			while(std::getline(fIn, s))
			{
				input += s; input += '\n';
			}
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
			else {
				std::cout << "parse failed" << std::endl;
			}
		}
		catch (const boost::bad_any_cast& e)
		{
			std::cout << e.what() << std::endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
