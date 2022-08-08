#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "ast_noh.hpp"

namespace Noh {
namespace eval {

struct AstEval {
	std::int_fast64_t ifInFunc = 0;
	std::int_fast64_t ifInWhile = 0;
	bool ifExit = false;
	bool ifReturn = false;
	bool ifBreak = false;
	bool ifContinue = false;
	std::unordered_set<std::string> builtin = {
		"break",
		"continue",
		"exit",
		"return",
		"print",
		"scan",

		// "var",
		"fn",
		"if",
		"then",
		"else",
		"end",
		"while"
	};
	std::unordered_map<std::string, std::int_fast64_t> i64Vals;
	std::unordered_map<std::string, std::string> stringVals;
	std::unordered_map<std::string, ast::FuncAst*> funcs;

	AstEval(ast::ModuleAst* ast) { evalModuleAst(ast); }
	~AstEval()
	{
		builtin.clear();
		i64Vals.clear();
		stringVals.clear();
		funcs.clear();
	}

	void evalModuleAst(ast::ModuleAst* ast)
	{
		for(auto& func : ast->getFuncs())
		{
			const auto& funcName = func->getName();
			assert(funcs.find(funcName) == std::end(funcs));
			assert(builtin.find(funcName) == std::end(builtin));
			funcs[funcName] = func;
		}
		if(funcs.find(std::string("main")) != std::end(funcs)) { evalFuncAst(funcs["main"]); }
		else { assert(0 && "not found \"fn main\""); }
	}

	void evalFuncAst(ast::FuncAst* ast)
	{
		if(ifExit)return;
		for(auto& stmts : ast->getInst())
		{
			evalStmts(stmts);
			if(ifExit)return;
			if(ifReturn) { ifReturn = false; break; }
		}
	}

	bool ifExistini64Vals(const std::string& s) { return i64Vals.find(s) != std::end(i64Vals); }
	bool ifExistinStringVals(const std::string& s) { return stringVals.find(s) != std::end(stringVals); }

	void evalStmts(ast::BaseAst* ast)
	{
		if(ifExit)return;
		if(ast->getID() == ast::BuiltinID)
		{
			evalBuiltinAst(static_cast<ast::BuiltinAst*>(ast));
		}
		else if(ast->getID() == ast::AssignID)
		{
			evalAssignAst(static_cast<ast::AssignAst*>(ast));
		}
		else if(ast->getID() == ast::IfStmtID)
		{
			evalIfStmtAst(static_cast<ast::IfStmtAst*>(ast));
		}
		else if(ast->getID() == ast::WhileStmtID)
		{
			evalWhileStmtAst(static_cast<ast::WhileStmtAst*>(ast));
		}
		else if(ast->getID() == ast::ForStmtID)
		{
			evalForStmtAst(static_cast<ast::ForStmtAst*>(ast));
		}
		else
		{
			assert(0 && "illegal ast");
		}
	}

	void evalBuiltinAst(ast::BuiltinAst* ast)
	{
		if(ifExit)return;
		const auto& builtinName = ast->getName();
		if(builtinName == "break")ifBreak = true;
		else if(builtinName == "continue")ifContinue = true;
		else if(builtinName == "exit")ifExit = true;
		else if(builtinName == "return")ifReturn = true;
		else if(builtinName == "print")
		{
			for(auto& arg : ast->Args)
			{
				const auto& ident = static_cast<ast::IdentAst*>(arg)->getIdent();
				if(ifExistini64Vals(ident))
				{
					std::cout << i64Vals.at(ident) << std::endl;
				}
				else if(ifExistinStringVals(ident))
				{
					std::cout << stringVals.at(ident) << std::endl;
				}
				else
				{
					assert(0 && "undefined ident");
				}
			}
		}
		else if(builtinName == "scani")
		{
			for(auto& arg : ast->Args)
			{
				const auto& ident = static_cast<ast::IdentAst*>(arg)->getIdent();
				if(ifExistinStringVals(ident))
				{
					assert(0 && "cannot convert int to string");
				}
				std::cin >> i64Vals[ident];
			}
		}
		else if(builtinName == "scans")
		{
			for(auto& arg : ast->Args)
			{
				const auto& ident = static_cast<ast::IdentAst*>(arg)->getIdent();
				if(ifExistinStringVals(ident))
				{
					assert(0 && "cannot convert string to int");
				}
				std::cin >> stringVals[ident];
			}
		}
	}

	void evalAssignAst(ast::AssignAst* ast)
	{
		if(ifExit)return;
		if(ast->getVal()->getID() == ast::StringID)
		{
			if(ifExistini64Vals(ast->getName()))
			{
				assert(0 && "cannot convert string to int");
			}
			else
			{
				stringVals[ast->getName()] = static_cast<ast::StringAst*>(ast->getVal())->getVal();
			}
		}
		else
		{
			if(ifExistinStringVals(ast->getName()))
			{
				assert(0 && "cannot convert int to string");
			}
			else
			{
				i64Vals[ast->getName()] = evalExpr(ast->getVal());
			}
		}
	}

	void evalIfStmtAst(ast::IfStmtAst* ast)
	{
		if(ifExit)return;
		const auto CondEval = evalExpr(ast->Cond);
		if(CondEval)
		{
			for(auto& stmt : ast->getThenStmt())
			{
				evalStmts(stmt);
			}
		}
		else
		{
			if(not ast->ElseStmt.empty())
			{
				for(auto& stmt : ast->getElseStmt())
				{
					evalStmts(stmt);
				}
			}
		}
	}

	void evalWhileStmtAst(ast::WhileStmtAst* ast)
	{
		if(ifExit)return;
		while(evalExpr(ast->getCond()))
		{
			for(auto& stmt : ast->getLoopStmt())
			{
				evalStmts(stmt);
				if(ifBreak)
				{
					ifBreak = false;
					return;
				}
				if(ifContinue)
				{
					ifContinue = false;
					break;
				}
				if(ifExit)return;
			}
		}
	}

	void evalForStmtAst(ast::ForStmtAst* ast)
	{
		if(ifExit)return;
		const auto& name = ast->getIdent();
		bool existVal = i64Vals.find(name) != std::end(i64Vals);
		std::int_fast64_t tmp{};
		if(existVal) { tmp = i64Vals[name]; }

		for(i64Vals[name] = evalExpr(ast->getRange()->getFrom()); \
			i64Vals[name] < evalExpr(ast->getRange()->getTo()); i64Vals[name]++)
		{
			for(auto& stmt : ast->getStmts())
			{
				evalStmts(stmt);
				if(ifBreak)
				{
					ifBreak = false;
					return;
				}
				if(ifContinue)
				{
					ifContinue = false;
					break;
				}
				if(ifExit)return;
			}
		}

		if(existVal) { i64Vals[name] = tmp; }
		else { i64Vals.erase(name); }
	}

	std::int_fast64_t evalExpr(ast::BaseAst* ast)
	{
		if(ast->getID() == ast::NumberID)
		{
			return evalNumberAst(static_cast<ast::NumberAst*>(ast));
		}
		else if(ast->getID() == ast::IdentID)
		{
			return i64Vals.at(static_cast<ast::IdentAst*>(ast)->getIdent());
		}
		else if(ast->getID() == ast::BinaryExpID)
		{
			return evalBinaryExpAst(static_cast<ast::BinaryExpAst*>(ast));
		}
		else if(ast->getID() == ast::MonoExpID)
		{
			return evalMonoExpAst(static_cast<ast::MonoExpAst*>(ast));
		}
		else // otherwize
		{
			assert(0 && "no match");
		}
	}

	std::int_fast64_t evalNumberAst(ast::NumberAst* ast)
	{
		return ast->getVal();
	}

	std::int_fast64_t evalBinaryExpAst(ast::BinaryExpAst* ast)
	{
		bool isLhsNumber = ast->getLhs()->getID() == ast::NumberID,
			 isRhsNumber = ast->getRhs()->getID() == ast::NumberID,
			 isLhsMonoExp = ast->getLhs()->getID() == ast::MonoExpID,
			 isRhsMonoExp = ast->getRhs()->getID() == ast::MonoExpID,
			 isLhsBinExp = ast->getLhs()->getID() == ast::BinaryExpID,
			 isRhsBinExp = ast->getRhs()->getID() == ast::BinaryExpID,
			 isLhsIdent = ast->getLhs()->getID() == ast::IdentID,
			 isRhsIdent = ast->getRhs()->getID() == ast::IdentID;
		const auto LhsEval =
			isLhsNumber ? evalNumberAst(static_cast<ast::NumberAst*>(ast->getLhs())) : \
			isLhsMonoExp ? evalMonoExpAst(static_cast<ast::MonoExpAst*>(ast->getLhs())) : \
			isLhsBinExp ? evalBinaryExpAst(static_cast<ast::BinaryExpAst*>(ast->getLhs())) : \
			isLhsIdent ? i64Vals.at(static_cast<ast::IdentAst*>(ast->getLhs())->getIdent()) : \
			-1; // dummy
		const auto RhsEval =
			isRhsNumber ? evalNumberAst(static_cast<ast::NumberAst*>(ast->getRhs())) : \
			isRhsMonoExp ? evalMonoExpAst(static_cast<ast::MonoExpAst*>(ast->getRhs())) : \
			isRhsBinExp ? evalBinaryExpAst(static_cast<ast::BinaryExpAst*>(ast->getRhs())) : \
			isRhsIdent ? i64Vals.at(static_cast<ast::IdentAst*>(ast->getRhs())->getIdent()) : \
			-1; // dummy
		if(ast->getOp() == "+") { return LhsEval + RhsEval; }
		else if(ast->getOp() == "-") { return LhsEval - RhsEval; }
		else if(ast->getOp() == "*") { return LhsEval * RhsEval; }
		else if(ast->getOp() == "/") { return LhsEval / RhsEval; }
		else if(ast->getOp() == "%") { return LhsEval % RhsEval; }
		else if(ast->getOp() == "==") { return static_cast<std::int_fast64_t>(LhsEval == RhsEval); }
		else if(ast->getOp() == "!=") { return static_cast<std::int_fast64_t>(LhsEval != RhsEval); }
		else if(ast->getOp() == "<") { return static_cast<std::int_fast64_t>(LhsEval < RhsEval); }
		else if(ast->getOp() == ">") { return static_cast<std::int_fast64_t>(LhsEval > RhsEval); }
		else if(ast->getOp() == "<=") { return static_cast<std::int_fast64_t>(LhsEval <= RhsEval); }
		else if(ast->getOp() == ">=") { return static_cast<std::int_fast64_t>(LhsEval >= RhsEval); }
		else if(ast->getOp() == "&&") { return static_cast<std::int_fast64_t>(LhsEval && RhsEval); }
		else if(ast->getOp() == "||") { return static_cast<std::int_fast64_t>(LhsEval || RhsEval); }
		else // otherwize
		{
			assert(0 && "no match");
		}
		return 0;
	}

	std::int_fast64_t evalMonoExpAst(ast::MonoExpAst* ast)
	{
		bool isLhsNumber = ast->getLhs()->getID() == ast::NumberID,
			 isLhsMonoExp = ast->getLhs()->getID() == ast::MonoExpID,
			 isLhsBinExp = ast->getLhs()->getID() == ast::BinaryExpID,
			 isLhsIdent = ast->getLhs()->getID() == ast::IdentID;
			 
		const auto LhsEval =
			isLhsNumber ? evalNumberAst(static_cast<ast::NumberAst*>(ast->getLhs())) : \
			isLhsMonoExp ? evalMonoExpAst(static_cast<ast::MonoExpAst*>(ast->getLhs())) : \
			isLhsBinExp ? evalBinaryExpAst(static_cast<ast::BinaryExpAst*>(ast->getLhs())) : \
			isLhsIdent ? i64Vals.at(static_cast<ast::IdentAst*>(ast->getLhs())->getIdent()) : \
			-1; // dummy
		if(ast->getOp() == "!") { return static_cast<std::int_fast64_t>(! LhsEval); }
		else // otherwize
		{
			assert(0 && "no match");
		}
		return 0;
	}
};

} // namespace eval
} // namespace Noh
