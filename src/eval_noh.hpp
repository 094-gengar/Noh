#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <deque>

#include "ast_noh.hpp"

namespace Noh {
namespace eval {

struct AstEval {
	bool ExitFlag = false;
	bool ReturnFlag = false;
	bool BreakFlag = false;
	bool ContinueFlag = false;
	std::int_fast32_t valsSize, FuncRecSize, curLower;

	std::unordered_set<std::string> builtin = {
		"break",
		"continue",
		"exit",
		"return",
		"print",
		"scani",
		"scans",

		"var",
		"num",
		"str",
		"fn",
		"if",
		"then",
		"else",
		"end",
		"while",
		"for"
	};

	std::deque<std::unordered_map<std::string, ast::BaseAst*>> vals;
	std::unordered_map<std::string, ast::FuncAst*> funcs;

	AstEval(ast::ModuleAst*& ast) : valsSize(0), FuncRecSize(0), curLower(0)
	{
		evalModuleAst(ast);
	}
	~AstEval()
	{
		builtin.clear();
		vals.clear();
		funcs.clear();
	}

	bool CanCastInNum(ast::BaseAst* ast)
	{
		switch(ast->getID()) {
		case ast::NumberID:
			return true;
		case ast::BinaryExpID:
			return true;
		case ast::MonoExpID:
			return true;
		default:
			return false;
		}
		return false;		
	}

	bool CanCastInNum(const ast::AstID& id)
	{
		switch(id) {
		case ast::NumberID:
			return true;
		case ast::BinaryExpID:
			return true;
		case ast::MonoExpID:
			return true;
		default:
			return false;
		}
		return false;		
	}

	bool CanCastInStr(ast::BaseAst* ast)
	{
		switch(ast->getID()) {
		case ast::StringID:
			return true;
		default:
			return false;
		}
		return false;
	}

	bool CanCastInStr(const ast::AstID& id)
	{
		switch(id) {
		case ast::StringID:
			return true;
		default:
			return false;
		}
		return false;
	}

	std::int_fast8_t CastAstIdx(ast::BaseAst* ast)
	{
		if(CanCastInNum(ast))
		{
			return 0;
		}
		else if(CanCastInStr(ast))
		{
			return 1;
		}
		return -1;	
	}

	std::int_fast8_t CastAstIdx(const ast::AstID& id)
	{
		if(CanCastInNum(id))
		{
			return 0;
		}
		else if(CanCastInStr(id))
		{
			return 1;
		}
		return -1;	
	}

	ast::AstID TypeOfIdentAst(ast::IdentAst* ast)
	{
		const auto& name = ast->getIdent();
		assert(valsSize >= 1);
		for(std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
		{
			if(vals[i].find(name) != std::end(vals[i]))
			{
				const auto& id = vals[i].at(name)->getID();
				return id;
			}
		}
		assert(0);
	}

	ast::AstID TypeOfIdentAst(ast::BaseAst* ast)
	{
		if(ast->getID() != ast::IdentID)
		{
			return ast::NoneID;
		}
		const auto& name = dynamic_cast<ast::IdentAst*>(ast)->getIdent();
		assert(valsSize >= 1);
		for(std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
		{
			if(vals[i].find(name) != std::end(vals[i]))
			{
				const auto& id = vals[i].at(name)->getID();
				return id;
			}
		}
		assert(0);
	}

	// ================
	//      module
	// ================

	void evalModuleAst(ast::ModuleAst*& ast)
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

	// ================
	//       func
	// ================

	void evalFuncAst(ast::FuncAst*& ast, std::vector<ast::BaseAst*> params)
	{
		if(ExitFlag)return;
		const auto& siz = params.size();
		assert(siz == ast->getParamNames().size());
		auto paramNames = ast->getParamNames();

		vals.emplace_back(std::unordered_map<std::string, ast::BaseAst*>{});
		valsSize++;
		auto tmpLower = curLower;
		curLower = valsSize - 1;

		for(size_t i{}; i < siz; i++)
		{
			auto tmp = new ast::AssignAst{paramNames[i]};
			tmp->Val = params[i];
			evalAssignAst(tmp);
		}

		{
			for(auto& stmts : ast->getInst())
			{
				evalStmts(stmts);
				if(ExitFlag)return;
				if(ReturnFlag) { ReturnFlag = false; break; }
			}
		}

		vals.back().clear();
		vals.pop_back();
		valsSize--;
		curLower = tmpLower;
	}

	void evalCallAst(ast::CallAst* ast)
	{
		if(ExitFlag)return;
		const auto& fnName = ast->getFuncName();
		evalFuncAst(funcs[fnName], ast->getParams());
	}

	// ================
	//      stmts
	// ================

	void evalStmts(ast::BaseAst* ast)
	{
		if(ExitFlag)return;
		if(ast->getID() == ast::BuiltinID)
		{
			evalBuiltinAst(dynamic_cast<ast::BuiltinAst*>(ast));
		}
		else if(ast->getID() == ast::AssignID)
		{
			evalAssignAst(dynamic_cast<ast::AssignAst*>(ast));
		}
		else if(ast->getID() == ast::IfStmtID)
		{
			evalIfStmtAst(dynamic_cast<ast::IfStmtAst*>(ast));
		}
		else if(ast->getID() == ast::WhileStmtID)
		{
			evalWhileStmtAst(dynamic_cast<ast::WhileStmtAst*>(ast));
		}
		else if(ast->getID() == ast::ForStmtID)
		{
			evalForStmtAst(dynamic_cast<ast::ForStmtAst*>(ast));
		}
		else if(ast->getID() == ast::CallID)
		{
			evalCallAst(dynamic_cast<ast::CallAst*>(ast));
		}
		else
		{
			assert(0 && "illegal ast");
		}
	}

	// ================
	//     builtin
	// ================

	void evalBuiltinAst(ast::BuiltinAst* ast)
	{
		if(ExitFlag)return;
		const auto& builtinName = ast->getName();
		if(builtinName == "break")BreakFlag = true;
		else if(builtinName == "continue")ContinueFlag = true;
		else if(builtinName == "exit")ExitFlag = true;
		else if(builtinName == "return")ReturnFlag = true;
		else if(builtinName == "print")
		{
			for(auto& arg : ast->Args)
			{
				if(arg->getID() == ast::IdentID)
				{
					if(TypeOfIdentAst(arg) == ast::NumberID)
					{
						std::cout << evalNumExpr(arg) << std::endl;;
					}
					else if(TypeOfIdentAst(arg) == ast::StringID)
					{
						std::cout << evalStrExpr(arg) << std::endl;;
					}
					else
					{
						assert(0 && "unknown type");
					}
				}
				else
				{
					if(CanCastInNum(arg))
					{
						std::cout << evalNumExpr(arg) << std::endl;;
					}
					else if(CanCastInStr(arg))
					{
						std::cout << evalStrExpr(arg) << std::endl;;
					}
					else
					{
						assert(0 && "unknown type");
					}
				}
			}
		}
		else if(builtinName == "scani")
		{
			if(ExitFlag)return;
			for(auto& arg : ast->Args)
			{
				const auto& ident = dynamic_cast<ast::IdentAst*>(arg)->getIdent();

				bool flg = true;
				assert(valsSize >= 1);
				for(std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if(vals[i].find(ident) != std::end(vals[i]))
					{
						if(CanCastInNum(vals[i].at(ident)))
						{
							std::int_fast64_t tmp;
							std::cin >> tmp;
							dynamic_cast<ast::NumberAst*>(vals[i].at(ident))->getVal() = tmp;
							flg = false;
							break;
						}
						else assert(0 && "different type");
					}
				}
				if(flg) { assert(0 && "unknown ident"); }
			}
		}
		else if(builtinName == "scans")
		{
			if(ExitFlag)return;
			for(auto& arg : ast->Args)
			{
				const auto& ident = dynamic_cast<ast::IdentAst*>(arg)->getIdent();
	
				assert(valsSize >= 1);
				bool flg = true;
				for(std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if(vals[i].find(ident) != std::end(vals[i]))
					{
						if(CanCastInStr(vals[i].at(ident)))
						{
							std::string tmp;
							std::cin >> tmp;
							dynamic_cast<ast::StringAst*>(vals[i].at(ident))->getVal() = tmp;
							flg = false;
							break;
						}
						else assert(0 && "different type");
					}
				}
				if(flg) { assert(0 && "unknown ident"); }
			}
		}
	}

	// ================
	//      assign
	// ================

	void evalAssignAst(ast::AssignAst* ast)
	{
		if(ExitFlag)return;
		assert(builtin.find(ast->getName()) == std::end(builtin));
		const auto& valAst = ast->getVal();
		assert(not vals.empty());

		if(valAst->getID() == ast::IdentID)
		{
			if(TypeOfIdentAst(valAst) == ast::NumberID)
			{
				bool flg = true;
				for(std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					/*
					if(CastAstIdx(vals.back().at(ast->getName())) != CastAstIdx(valAst) and \
						CastAstIdx(vals.back().at(ast->getName())) != CastAstIdx(TypeOfIdentAst(valAst)))
					{
						std::cerr << CastAstIdx(vals.back().at(ast->getName())) << std::endl;
						std::cerr << CastAstIdx(valAst) << std::endl;
						std::cerr << valAst->getID() << std::endl;
						assert(0 && "different type");
					}
						*/
					if(vals[i].find(ast->getName()) != std::end(vals[i]))
					{
						if(CanCastInNum(vals[i].at(ast->getName())))
						{
							dynamic_cast<ast::NumberAst*>(vals[i].at(ast->getName()))->getVal() = evalNumExpr(valAst);
							flg = false;
							break;
						}
						else assert(0 && "different type");
					}
				}
				if(flg)
				{
					vals.back()[ast->getName()] = new ast::NumberAst(evalNumExpr(valAst));
				}
			}
			else if(TypeOfIdentAst(valAst) == ast::StringID)
			{
				bool flg = true;
				for(std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if(vals[i].find(ast->getName()) != std::end(vals[i]))
					{
						if(CanCastInStr(vals[i].at(ast->getName())))
						{
							dynamic_cast<ast::StringAst*>(vals[i].at(ast->getName()))->getVal() = evalStrExpr(valAst);
							flg = false;
							break;
						}
						else assert(0 && "different type");
					}
				}
				if(flg)
				{
					vals.back()[ast->getName()] = new ast::StringAst(evalStrExpr(valAst));
				}
			}
			else
			{
				assert(0 && "unknown type");
			}
		}
		else
		{
			if(CanCastInNum(valAst))
			{
				bool flg = true;
				for(std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if(vals[i].find(ast->getName()) != std::end(vals[i]))
					{
						if(CanCastInNum(vals[i].at(ast->getName())))
						{
							dynamic_cast<ast::NumberAst*>(vals[i].at(ast->getName()))->getVal() = evalNumExpr(valAst);
							flg = false;
							break;
						}
						else assert(0 && "different type");
					}
				}
				if(flg)
				{
					vals.back()[ast->getName()] = new ast::NumberAst(evalNumExpr(valAst));
				}
			}
			else if(CanCastInStr(valAst))
			{
				bool flg = true;
				for(std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if(vals[i].find(ast->getName()) != std::end(vals[i]))
					{
						if(CanCastInStr(vals[i].at(ast->getName())))
						{
							dynamic_cast<ast::StringAst*>(vals[i].at(ast->getName()))->getVal() = evalStrExpr(valAst);
							flg = false;
							break;
						}
						else assert(0 && "different type");
					}
				}
				if(flg)
				{
					vals.back()[ast->getName()] = new ast::StringAst(evalStrExpr(valAst));
				}
			}
			else
			{
				assert(0 && "unknown type");
			}
		}
	}

	// ================
	//        if
	// ================

	void evalIfStmtAst(ast::IfStmtAst* ast)
	{
		if(ExitFlag)return;
		assert(CanCastInNum(ast->Cond));

		vals.emplace_back(std::unordered_map<std::string, ast::BaseAst*>{});
		valsSize++;

		const auto CondEval = evalNumExpr(ast->Cond);
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

		vals.back().clear();
		vals.pop_back();
		valsSize--;
	}

	// ================
	//      while
	// ================

	void evalWhileStmtAst(ast::WhileStmtAst* ast)
	{
		if(ExitFlag)return;
		assert(CanCastInNum(ast->getCond()));

		vals.emplace_back(std::unordered_map<std::string, ast::BaseAst*>{});
		valsSize++;

		while(true)
		{
			const auto CondEval = evalNumExpr(ast->getCond());
			if(not CondEval)break;
			for(auto& stmt : ast->getLoopStmt())
			{
				evalStmts(stmt);
				if(BreakFlag)
				{
					BreakFlag = false;
					return;
				}
				if(ContinueFlag)
				{
					ContinueFlag = false;
					break;
				}
				if(ExitFlag)return;
			}
		}

		vals.back().clear();
		vals.pop_back();
		valsSize--;
	}

	// ================
	//       for
	// ================

	void evalForStmtAst(ast::ForStmtAst* ast)
	{
		if(ExitFlag)return;
		const auto& name = ast->getIdent();
		assert(builtin.find(name) == std::end(builtin));
		assert(CanCastInNum(ast->getRange()->getFrom()) and CanCastInNum(ast->getRange()->getTo()));

		vals.emplace_back(std::unordered_map<std::string, ast::BaseAst*>{});
		valsSize++;
		const auto fromEval = evalNumExpr(ast->getRange()->getFrom());
		const auto toEval = evalNumExpr(ast->getRange()->getTo());

		vals.back()[name] = new ast::NumberAst(fromEval);
		for(; dynamic_cast<ast::NumberAst*>(vals.back().at(name))->getVal() < toEval;)
		{
			for(auto& stmt : ast->getStmts())
			{
				evalStmts(stmt);
				if(BreakFlag)
				{
					BreakFlag = false;
					return;
				}
				if(ContinueFlag)
				{
					ContinueFlag = false;
					break;
				}
				if(ExitFlag)return;
			}

			dynamic_cast<ast::NumberAst*>(vals.back().at(name))->getVal()++;
		}

		vals.back().clear();
		vals.pop_back();
		valsSize--;
	}

	// ================
	//      string
	// ================

	std::string evalStrExpr(ast::BaseAst* ast)
	{
		if(ast->getID() == ast::StringID)
		{
			return evalStringAst(dynamic_cast<ast::StringAst*>(ast));
		}
		else if(ast->getID() == ast::IdentID)
		{
			const auto& name = dynamic_cast<ast::IdentAst*>(ast)->getIdent();

			assert(valsSize >= 1);
			for(std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
			{
				if(vals[i].find(name) != std::end(vals[i]))
				{
					if(CanCastInStr(vals[i].at(name)))
					{
						return evalStrExpr(vals[i].at(name));
					}
					else
					{
						assert(0);
					}
				}
			}
			assert(0 && "unknown ident");
		}
		else // otherwise
		{
			return std::string("!match");
		}
		return std::string("!match");
	}

	std::string evalStringAst(ast::StringAst* ast)
	{
		return ast->getVal();
	}

	// ================
	//      Number
	// ================

	std::int_fast64_t evalNumExpr(ast::BaseAst* ast)
	{
		if(ast->getID() == ast::NumberID)
		{
			return evalNumberAst(dynamic_cast<ast::NumberAst*>(ast));
		}
		else if(ast->getID() == ast::IdentID)
		{
			const auto& name = dynamic_cast<ast::IdentAst*>(ast)->getIdent();

			assert(valsSize >= 1);
			for(std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
			{
				if(vals[i].find(name) != std::end(vals[i]))
				{
					if(CanCastInNum(vals[i].at(name)))
					{
						return evalNumExpr(vals[i].at(name));
					}
					else
					{
						assert(0);
					}
				}
			}

			assert(0 && "unknown ident");
		}
		else if(ast->getID() == ast::BinaryExpID)
		{
			return evalBinaryExpAst(dynamic_cast<ast::BinaryExpAst*>(ast));
		}
		else if(ast->getID() == ast::MonoExpID)
		{
			return evalMonoExpAst(dynamic_cast<ast::MonoExpAst*>(ast));
		}
		else // otherwize
		{
			return -1;
		}
		return -1;
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
			isLhsNumber ? evalNumberAst(dynamic_cast<ast::NumberAst*>(ast->getLhs())) : \
			isLhsMonoExp ? evalMonoExpAst(dynamic_cast<ast::MonoExpAst*>(ast->getLhs())) : \
			isLhsBinExp ? evalBinaryExpAst(dynamic_cast<ast::BinaryExpAst*>(ast->getLhs())) : \
			isLhsIdent ? evalNumExpr(dynamic_cast<ast::IdentAst*>(ast->getLhs())) : \
			-1; // dummy
		const auto RhsEval =
			isRhsNumber ? evalNumberAst(dynamic_cast<ast::NumberAst*>(ast->getRhs())) : \
			isRhsMonoExp ? evalMonoExpAst(dynamic_cast<ast::MonoExpAst*>(ast->getRhs())) : \
			isRhsBinExp ? evalBinaryExpAst(dynamic_cast<ast::BinaryExpAst*>(ast->getRhs())) : \
			isRhsIdent ? evalNumExpr(dynamic_cast<ast::IdentAst*>(ast->getRhs())) : \
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
			isLhsNumber ? evalNumberAst(dynamic_cast<ast::NumberAst*>(ast->getLhs())) : \
			isLhsMonoExp ? evalMonoExpAst(dynamic_cast<ast::MonoExpAst*>(ast->getLhs())) : \
			isLhsBinExp ? evalBinaryExpAst(dynamic_cast<ast::BinaryExpAst*>(ast->getLhs())) : \
			isLhsIdent ? evalNumExpr(dynamic_cast<ast::IdentAst*>(ast->getLhs())) : \
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
