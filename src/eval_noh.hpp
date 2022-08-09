#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include <utility>
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
		"scani",
		"scans",

		"var"
		"fn",
		"if",
		"then",
		"else",
		"end",
		"while",
		"for"
	};
	std::unordered_map<std::string, std::int_fast64_t> i64Vals;
	std::unordered_map<std::string, std::string> StringVals;
	std::unordered_map<std::string, ast::FuncAst*> funcs;

	AstEval(ast::ModuleAst* ast) { evalModuleAst(ast); }
	~AstEval()
	{
		builtin.clear();
		i64Vals.clear();
		StringVals.clear();
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

		if(funcs.find(std::string("main")) != std::end(funcs))
		{
			assert(funcs.at("main")->getParamNames().size() == 0);
			evalFuncAst(funcs.at("main"), std::vector<ast::BaseAst*>{});
		}
		else
		{
			for(auto& func : ast->getFuncs())
			{
				if(func->getParamNames().size() == 0)
				{
					evalFuncAst(func, std::vector<ast::BaseAst*>{});
					break;
				}
			}
		}
	}

	void evalFuncAst(ast::FuncAst* ast, std::vector<ast::BaseAst*> params)
	{
		if(ifExit)return;
		ifInFunc++;
		assert(params.size() == ast->getParamNames().size());
		auto paramNames = ast->getParamNames();
		const auto& siz = params.size();
		std::unordered_map<std::string, std::int_fast64_t> tmpi64Vals;
		std::unordered_map<std::string, std::string> tmpStringVals;

		for(size_t i{}; i < siz; i++)
		{
			if(auto r1 = evalExpr(params[i]); r1.second)
			{
				tmpi64Vals[paramNames[i]] = r1.first;
			}
			else if(auto r2 = evalStrExpr(params[i]); r2.second)
			{
				tmpStringVals[paramNames[i]] = r2.first;
			}
			else
			{
				assert(0 && "illegal argument");
			}
		}

		std::swap(i64Vals, tmpi64Vals);
		std::swap(StringVals, tmpStringVals);

		{
			for(auto& stmts : ast->getInst())
			{
				evalStmts(stmts);
				if(ifExit)return;
				if(ifReturn) { ifReturn = false; break; }
			}
		}

		std::swap(i64Vals, tmpi64Vals);
		std::swap(StringVals, tmpStringVals);
		tmpi64Vals.clear();
		tmpStringVals.clear();
		ifInFunc--;
	}

	void evalCallAst(ast::CallAst* ast)
	{
		if(ifExit)return;
		const auto& fnName = ast->getFuncName();
		evalFuncAst(funcs[fnName], ast->getParams());
	}

	bool ifExistini64Vals(const std::string& s) { return i64Vals.find(s) != std::end(i64Vals); }
	bool ifExistinStringVals(const std::string& s) { return StringVals.find(s) != std::end(StringVals); }

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
		else if(ast->getID() == ast::CallID)
		{
			evalCallAst(static_cast<ast::CallAst*>(ast));
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
				if(auto r1 = evalExpr(arg); r1.second)
				{
					std::cout << r1.first << std::endl;
				}
				else if(auto r2 = evalStrExpr(arg); r2.second)
				{
					std::cout << r2.first << std::endl;
				}
				else
				{
					assert(0 && "illegal argument");
				}
			}
		}
		else if(builtinName == "scani")
		{
			if(ifExit)return;
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
			if(ifExit)return;
			for(auto& arg : ast->Args)
			{
				const auto& ident = static_cast<ast::IdentAst*>(arg)->getIdent();
				if(ifExistinStringVals(ident))
				{
					assert(0 && "cannot convert string to int");
				}
				std::cin >> StringVals[ident];
			}
		}
	}

	void evalAssignAst(ast::AssignAst* ast)
	{
		if(ifExit)return;
		const auto& ValAst = ast->getVal();
		if(auto r1 = evalExpr(ValAst); r1.second)
		{
			if(ifExistinStringVals(ast->getName()))
			{
				assert(0 && "cannot convert int to string");
			}
			i64Vals[ast->getName()] = r1.first;
		}
		else if(auto r2 = evalStrExpr(ValAst); r2.second)
		{
			if(ifExistini64Vals(ast->getName()))
			{
				assert(0 && "cannot convert string to int");
			}
			StringVals[ast->getName()] = r2.first;
		}
		else
		{
			assert(0 && "illegal argument");
		}
	}

	void evalIfStmtAst(ast::IfStmtAst* ast)
	{
		if(ifExit)return;
		const auto[CondEval, flg] = evalExpr(ast->Cond);
		assert(flg);
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
		ifInWhile++;
		while(true)
		{
			const auto[CondEval, flg] = evalExpr(ast->getCond());
			if(not (flg and CondEval))break;
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
		ifInWhile--;
	}

	void evalForStmtAst(ast::ForStmtAst* ast)
	{
		if(ifExit)return;
		const auto& name = ast->getIdent();
		bool existVal = i64Vals.find(name) != std::end(i64Vals);
		std::int_fast64_t tmp{};
		if(existVal) { tmp = i64Vals[name]; }
		const auto[fromEval, fromFlg] = evalExpr(ast->getRange()->getFrom());
		const auto[toEval, toFlg] = evalExpr(ast->getRange()->getTo());
		assert(fromFlg and toFlg);

		for(i64Vals[name] = fromEval; \
			i64Vals[name] < toEval; i64Vals[name]++)
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

	std::pair<std::string, bool> evalStrExpr(ast::BaseAst* ast)
	{
		if(ast->getID() == ast::StringID)
		{
			return std::make_pair(evalStringAst(static_cast<ast::StringAst*>(ast)), true);
		}
		else if(ast->getID() == ast::IdentID)
		{
			const auto& name = static_cast<ast::IdentAst*>(ast)->getIdent();
			if(not ifExistini64Vals(name))
			{
				return std::make_pair(StringVals.at(name), true);
			}
		}
		else // otherwise
		{
			return std::make_pair("!match", false);
		}
		return std::make_pair("!match", false);
	}

	std::string evalStringAst(ast::StringAst* ast)
	{
		return ast->getVal();
	}

	std::pair<std::int_fast64_t, bool> evalExpr(ast::BaseAst* ast)
	{
		if(ast->getID() == ast::NumberID)
		{
			return std::make_pair(evalNumberAst(static_cast<ast::NumberAst*>(ast)), true);
		}
		else if(ast->getID() == ast::IdentID)
		{
			const auto& name = static_cast<ast::IdentAst*>(ast)->getIdent();
			if(not ifExistinStringVals(name))
			{
				return std::make_pair(i64Vals.at(name), true);
			}
		}
		else if(ast->getID() == ast::BinaryExpID)
		{
			return std::make_pair(evalBinaryExpAst(static_cast<ast::BinaryExpAst*>(ast)), true);
		}
		else if(ast->getID() == ast::MonoExpID)
		{
			return std::make_pair(evalMonoExpAst(static_cast<ast::MonoExpAst*>(ast)), true);
		}
		else // otherwize
		{
			return std::make_pair(-1, false);
		}
		return std::make_pair(-1, false);
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
