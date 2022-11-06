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
	ast::BaseAst* ReturnValue = nullptr;
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
		switch (ast->getID()) {
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
		switch (id) {
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
		switch (ast->getID()) {
		case ast::StringID:
			return true;
		default:
			return false;
		}
		return false;
	}

	bool CanCastInStr(const ast::AstID& id)
	{
		switch (id) {
		case ast::StringID:
			return true;
		default:
			return false;
		}
		return false;
	}

	bool CanCastInTpl(const ast::BaseAst* ast)
	{
		return ast->getID() == ast::TupleID;
	}

	bool CanCastInTpl(const ast::AstID& id)
	{
		return id == ast::TupleID;
	}

	std::int_fast8_t CastAstIdx(ast::BaseAst* ast)
	{
		if (CanCastInNum(ast))
		{
			return 0;
		}
		else if (CanCastInStr(ast))
		{
			return 1;
		}
		else if (ast->getID() == ast::TupleID)
		{
			return 2;
		}
		return -1;	
	}

	std::int_fast8_t CastAstIdx(const ast::AstID& id)
	{
		if (CanCastInNum(id))
		{
			return 0;
		}
		else if (CanCastInStr(id))
		{
			return 1;
		}
		else if (id == ast::TupleID)
		{
			return 2;
		}
		return -1;	
	}

	ast::AstID TypeOfIdentAst(ast::IdentAst* ast)
	{
		const auto& name = ast->getIdent();
		assert(valsSize >= 1);
		for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
		{
			if (vals[i].find(name) != std::end(vals[i]))
			{
				const auto& id = vals[i].at(name)->getID();
				return id;
			}
		}
		assert(0);
	}

	ast::AstID TypeOfIdentAst(ast::BaseAst* ast)
	{
		if (ast->getID() != ast::IdentID)
		{
			return ast::NoneID;
		}
		const auto& name = dynamic_cast<ast::IdentAst*>(ast)->getIdent();
		assert(valsSize >= 1);
		for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
		{
			if (vals[i].find(name) != std::end(vals[i]))
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
		for (auto& func : ast->getFuncs())
		{
			const auto& funcName = func->getName();
			assert(funcs.find(funcName) == std::end(funcs));
			assert(builtin.find(funcName) == std::end(builtin));
			funcs[funcName] = func;
		}
		if (funcs.find(std::string("main")) != std::end(funcs))
		{
			assert(funcs.at("main")->getParamNames().size() == 0);
			evalFuncAst(funcs.at("main"), std::vector<ast::BaseAst*>{});
		}
		else
		{
			for (auto& func : ast->getFuncs())
			{
				if (func->getParamNames().size() == 0)
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

	ast::BaseAst* evalFuncAst(ast::FuncAst*& ast, std::vector<ast::BaseAst*> params)
	{
		if (ExitFlag) { return nullptr; }
		const auto& siz = params.size();
		assert(siz == ast->getParamNames().size());
		auto paramNames = ast->getParamNames();

		vals.emplace_back(std::unordered_map<std::string, ast::BaseAst*>{});
		valsSize++;

		for (size_t i{}; i < siz; i++)
		{
			auto tmp = new ast::AssignAst{paramNames[i]};
			if (params[i]->getID() == ast::CallID)
			{
				while (params[i]->getID() == ast::CallID)
				{
					params[i] = evalCallAst(dynamic_cast<ast::CallAst*>(params[i]));
				}
			}
			tmp->Val = params[i];
			evalAssignAst(tmp);
		}

		auto tmpLower = curLower;
		curLower = valsSize - 1;


		ast::BaseAst* Ret = new ast::NumberAst(0);

		{
			for (auto& stmts : ast->getInst())
			{
				evalStmts(stmts);
				if (ExitFlag) { return nullptr; }
				if (ReturnFlag)
				{
					if (ReturnValue)
					{
						Ret = ReturnValue;
						ReturnValue = nullptr;
					}
					ReturnFlag = false;
					break;
				}
			}
		}

		if (Ret->getID() == ast::IdentID)
		{
			if (TypeOfIdentAst(Ret) == ast::NumberID)
			{
				Ret = new ast::NumberAst(evalNumExpr(Ret));
			}
			else if (TypeOfIdentAst(Ret) == ast::StringID)
			{
				Ret = new ast::StringAst(evalStrExpr(Ret));
			}
			else if (TypeOfIdentAst(Ret) == ast::TupleID)
			{
				Ret = new ast::TupleAst(evalTplExpr(Ret));
			}
			else assert(0 && "unknown type");
		}
		else
		{
			if (CanCastInNum(Ret))
			{
				Ret = new ast::NumberAst(evalNumExpr(Ret));
			}
			else if (CanCastInStr(Ret))
			{
				Ret = new ast::StringAst(evalStrExpr(Ret));
			}
			else if (CanCastInTpl(Ret))
			{
				Ret = new ast::TupleAst(evalTplExpr(Ret));
			}
			else if (Ret->getID() == ast::CallID)
			{
				Ret = evalCallAst(dynamic_cast<ast::CallAst*>(Ret));
			}
			else assert(0 && "unknown type");
		}

		vals.back().clear();
		vals.pop_back();
		valsSize--;
		curLower = tmpLower;
		return Ret;
	}

	ast::BaseAst* evalCallAst(ast::CallAst* ast)
	{
		if (ExitFlag) { return nullptr; }
		const auto& fnName = ast->getFuncName();
		if (funcs.find(fnName) == std::end(funcs) and ast->getParams().size() == 1)
		{
			return new ast::NumberAst(evalNumBinaryExpAst(new ast::BinaryExpAst(
					std::string("IdxAt"),
					new ast::IdentAst(ast->getFuncName()),
					ast->getParams().front())));
		}
		return evalFuncAst(funcs[fnName], ast->getParams());
	}

	// ================
	//      stmts
	// ================

	void evalStmts(ast::BaseAst* ast)
	{
		if (ExitFlag) { return; }
		if (ast->getID() == ast::BuiltinID)
		{
			evalBuiltinAst(dynamic_cast<ast::BuiltinAst*>(ast));
		}
		else if (ast->getID() == ast::AssignID)
		{
			evalAssignAst(dynamic_cast<ast::AssignAst*>(ast));
		}
		else if (ast->getID() == ast::ReAssignID)
		{
			evalReAssignAst(dynamic_cast<ast::ReAssignAst*>(ast));
		}
		else if (ast->getID() == ast::IfStmtID)
		{
			evalIfStmtAst(dynamic_cast<ast::IfStmtAst*>(ast));
		}
		else if (ast->getID() == ast::WhileStmtID)
		{
			evalWhileStmtAst(dynamic_cast<ast::WhileStmtAst*>(ast));
		}
		else if (ast->getID() == ast::ForStmtID)
		{
			evalForStmtAst(dynamic_cast<ast::ForStmtAst*>(ast));
		}
		else if (ast->getID() == ast::CallID)
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
		if (ExitFlag) { return; }
		const auto& builtinName = ast->getName();
		if (builtinName == "break")
		{
			BreakFlag = true;
		}
		else if (builtinName == "continue")
		{
			ContinueFlag = true;
		}
		else if (builtinName == "exit")
		{
			ExitFlag = true;
		}
		else if (builtinName == "return")
		{
			ReturnFlag = true;
			ReturnValue = ast->getArgs().front();
		}
		else if (builtinName == "print")
		{
			for (auto arg : ast->Args)
			{
				if (arg->getID() == ast::CallID)
				{
					arg = evalCallAst(dynamic_cast<ast::CallAst*>(arg));
				}
				
				if (arg->getID() == ast::IdentID)
				{
					if (TypeOfIdentAst(arg) == ast::NumberID)
					{
						std::cout << evalNumExpr(arg) << std::endl;
					}
					else if (TypeOfIdentAst(arg) == ast::StringID)
					{
						std::cout << evalStrExpr(arg) << std::endl;
					}
					else
					{
						assert(0 && "unknown type");
					}
				}
				else
				{
					if (CanCastInNum(arg))
					{
						std::cout << evalNumExpr(arg) << std::endl;
					}
					else if (CanCastInStr(arg))
					{
						std::cout << evalStrExpr(arg) << std::endl;
					}
					else
					{
						assert(0 && "unknown type");
					}
				}
			}
		}
		else if (builtinName == "scanNum")
		{
			if (ExitFlag) { return; }
			for (auto& arg : ast->Args)
			{
				const auto& ident = dynamic_cast<ast::IdentAst*>(arg)->getIdent();

				bool flg = true;
				assert(valsSize >= 1);
				for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if (vals[i].find(ident) != std::end(vals[i]))
					{
						if (CanCastInNum(vals[i].at(ident)))
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
				if (flg) { assert(0 && "unknown ident"); }
			}
		}
		else if (builtinName == "scanStr")
		{
			if (ExitFlag) { return; }
			for (auto& arg : ast->Args)
			{
				const auto& ident = dynamic_cast<ast::IdentAst*>(arg)->getIdent();
	
				assert(valsSize >= 1);
				bool flg = true;
				for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if (vals[i].find(ident) != std::end(vals[i]))
					{
						if (CanCastInStr(vals[i].at(ident)))
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
	//    (re)assign
	// ================

	void evalAssignAst(ast::AssignAst* ast)
	{
		if (ExitFlag) { return; }
		assert(builtin.find(ast->getName()) == std::end(builtin));
		if (ast->getVal()->getID() == ast::CallID)
		{
			while (ast->getVal()->getID() == ast::CallID)
			{
				ast->getVal() = evalCallAst(dynamic_cast<ast::CallAst*>(ast->getVal()));
			}
		}
		const auto& valAst = ast->getVal();
		assert(not vals.empty());

		if (valAst->getID() == ast::IdentID)
		{
			if (TypeOfIdentAst(valAst) == ast::NumberID)
			{
				if (vals.back().find(ast->getName()) != std::end(vals.back()))
				{
					assert(0 && "redefinition of variable is not allowed");
				}
				vals.back()[ast->getName()] = new ast::NumberAst(evalNumExpr(valAst));
			}
			else if (TypeOfIdentAst(valAst) == ast::StringID)
			{
				if (vals.back().find(ast->getName()) != std::end(vals.back()))
				{
					assert(0 && "redefinition of variable is not allowed");
				}
				vals.back()[ast->getName()] = new ast::StringAst(evalStrExpr(valAst));
			}
			else if (TypeOfIdentAst(valAst) == ast::TupleID)
			{
				if (vals.back().find(ast->getName()) != std::end(vals.back()))
				{
					assert(0 && "redefinition of variable is not allowed");
				}
				vals.back()[ast->getName()] = new ast::TupleAst(evalTplExpr(valAst));
			}
			else
			{
				assert(0 && "unknown type");
			}
		}
		else
		{
			if (CanCastInNum(valAst))
			{
				if (vals.back().find(ast->getName()) != std::end(vals.back()))
				{
					assert(0 && "redefinition of variable is not allowed");
				}
				vals.back()[ast->getName()] = new ast::NumberAst(evalNumExpr(valAst));
			}
			else if (CanCastInStr(valAst))
			{
				if (vals.back().find(ast->getName()) != std::end(vals.back()))
				{
					assert(0 && "redefinition of variable is not allowed");
				}
				vals.back()[ast->getName()] = new ast::StringAst(evalStrExpr(valAst));
			}
			else if (CanCastInTpl(valAst))
			{
				if (vals.back().find(ast->getName()) != std::end(vals.back()))
				{
					assert(0 && "redefinition of variable is not allowed");
				}
				vals.back()[ast->getName()] = new ast::TupleAst(evalTplExpr(valAst));
			}
			else
			{
				assert(0 && "unknown type");
			}
		}
	}

	void evalReAssignAst(ast::ReAssignAst* ast)
	{
		if (ExitFlag) { return; }
		assert(builtin.find(ast->getName()) == std::end(builtin));
		if (ast->getVal()->getID() == ast::CallID)
		{
			while (ast->getVal()->getID() == ast::CallID)
			{
				ast->getVal() = evalCallAst(dynamic_cast<ast::CallAst*>(ast->getVal()));
			}
		}
		const auto& valAst = ast->getVal();
		assert(not vals.empty());

		if (valAst->getID() == ast::IdentID)
		{
			if (TypeOfIdentAst(valAst) == ast::NumberID)
			{
				bool flg = true;
				for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if (vals[i].find(ast->getName()) != std::end(vals[i]))
					{
						if (CanCastInNum(vals[i].at(ast->getName())))
						{
							dynamic_cast<ast::NumberAst*>(vals[i].at(ast->getName()))->getVal() = evalNumExpr(valAst);
							flg = false;
							break;
						}
						else assert(0 && "different type");
					}
				}
				if (flg)
				{
					assert(0 && "unknown ident");
				}
			}
			else if (TypeOfIdentAst(valAst) == ast::StringID)
			{
				bool flg = true;
				for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if (vals[i].find(ast->getName()) != std::end(vals[i]))
					{
						if (CanCastInStr(vals[i].at(ast->getName())))
						{
							dynamic_cast<ast::StringAst*>(vals[i].at(ast->getName()))->getVal() = evalStrExpr(valAst);
							flg = false;
							break;
						}
						else assert(0 && "different type");
					}
				}
				if (flg)
				{
					assert(0 && "unknown ident");
				}
			}
			else if (TypeOfIdentAst(valAst) == ast::TupleID)
			{
				bool flg = true;
				for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if (vals[i].find(ast->getName()) != std::end(vals[i]))
					{
						if (CanCastInStr(vals[i].at(ast->getName())))
						{
							dynamic_cast<ast::TupleAst*>(vals[i].at(ast->getName()))->getAry() = evalTplExpr(valAst);
							flg = false;
							break;
						}
						else assert(0 && "different type");
					}
				}
				if (flg)
				{
					assert(0 && "unknown ident");
				}
			}
			else
			{
				assert(0 && "unknown type");
			}
		}
		else
		{
			if (CanCastInNum(valAst))
			{
				bool flg = true;
				for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if (vals[i].find(ast->getName()) != std::end(vals[i]))
					{
						if (CanCastInNum(vals[i].at(ast->getName())))
						{
							dynamic_cast<ast::NumberAst*>(vals[i].at(ast->getName()))->getVal() = evalNumExpr(valAst);
							flg = false;
							break;
						}
						else assert(0 && "different type");
					}
				}
				if (flg)
				{
					assert(0 && "unknown ident");
				}
			}
			else if (CanCastInStr(valAst))
			{
				bool flg = true;
				for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
				{
					if (vals[i].find(ast->getName()) != std::end(vals[i]))
					{
						if (CanCastInStr(vals[i].at(ast->getName())))
						{
							dynamic_cast<ast::StringAst*>(vals[i].at(ast->getName()))->getVal() = evalStrExpr(valAst);
							flg = false;
							break;
						}
						else assert(0 && "different type");
					}
				}
				if (flg)
				{
					assert(0 && "unknown ident");
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
		if (ExitFlag) { return; }
		assert(CanCastInNum(ast->Cond));

		vals.emplace_back(std::unordered_map<std::string, ast::BaseAst*>{});
		valsSize++;

		const auto CondEval = evalNumExpr(ast->Cond);
		if (CondEval)
		{
			for (auto& stmt : ast->getThenStmt())
			{
				evalStmts(stmt);
			}
		}
		else
		{
			if (not ast->ElseStmt.empty())
			{
				for (auto& stmt : ast->getElseStmt())
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
		if (ExitFlag) { return; }
		assert(CanCastInNum(ast->getCond()));

		vals.emplace_back(std::unordered_map<std::string, ast::BaseAst*>{});
		valsSize++;

		while (true)
		{
			const auto CondEval = evalNumExpr(ast->getCond());
			if (not CondEval) { break; }
			for (auto& stmt : ast->getLoopStmt())
			{
				evalStmts(stmt);
				if (BreakFlag)
				{
					BreakFlag = false;
					return;
				}
				if (ContinueFlag)
				{
					ContinueFlag = false;
					break;
				}
				if (ExitFlag) { return; }
			}
			vals.back().clear();
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
		if (ExitFlag) { return; }
		const auto& name = ast->getIdent();
		assert(builtin.find(name) == std::end(builtin));
		assert(CanCastInNum(ast->getRange()->getFrom()) or CanCastInNum(TypeOfIdentAst(ast->getRange()->getFrom())));
		assert(CanCastInNum(ast->getRange()->getTo()) or CanCastInNum(TypeOfIdentAst(ast->getRange()->getTo())));

		vals.emplace_back(std::unordered_map<std::string, ast::BaseAst*>{});
		valsSize++;
		const auto fromEval = evalNumExpr(ast->getRange()->getFrom());
		const auto toEval = evalNumExpr(ast->getRange()->getTo());

		auto tmpItr = fromEval;
		vals.back()[name] = new ast::NumberAst(fromEval);

		for (; tmpItr < toEval;)
		{
			for (auto& stmt : ast->getStmts())
			{
				evalStmts(stmt);
				if (BreakFlag)
				{
					BreakFlag = false;
					return;
				}
				if (ContinueFlag)
				{
					ContinueFlag = false;
					break;
				}
				if (ExitFlag) { return; }
			}

			vals.back().clear();
			vals.back()[name] = new ast::NumberAst(++tmpItr);
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
		if (ast->getID() == ast::StringID)
		{
			return evalStringAst(dynamic_cast<ast::StringAst*>(ast));
		}
		else if (ast->getID() == ast::IdentID)
		{
			const auto& name = dynamic_cast<ast::IdentAst*>(ast)->getIdent();

			assert(valsSize >= 1);
			for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
			{
				if (vals[i].find(name) != std::end(vals[i]))
				{
					if (CanCastInStr(vals[i].at(name)))
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
		if (ast->getID() == ast::NumberID)
		{
			return evalNumberAst(dynamic_cast<ast::NumberAst*>(ast));
		}
		else if (ast->getID() == ast::IdentID)
		{
			const auto& name = dynamic_cast<ast::IdentAst*>(ast)->getIdent();

			assert(valsSize >= 1);
			for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
			{
				if (vals[i].find(name) != std::end(vals[i]))
				{
					if (CanCastInNum(vals[i].at(name)))
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
		else if (ast->getID() == ast::BinaryExpID)
		{
			return evalNumBinaryExpAst(dynamic_cast<ast::BinaryExpAst*>(ast));
		}
		else if (ast->getID() == ast::MonoExpID)
		{
			return evalNumMonoExpAst(dynamic_cast<ast::MonoExpAst*>(ast));
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

	std::int_fast64_t evalNumBinaryExpAst(ast::BinaryExpAst* ast)
	{
		if (CanCastInTpl(ast->getLhs()) or CanCastInTpl(TypeOfIdentAst(ast->getLhs())))
		{
			const auto LhsEval = evalTplExpr(ast->getLhs());
			if (CanCastInNum(ast->getRhs()) or CanCastInNum(TypeOfIdentAst(ast->getRhs())))
			{
				assert(CanCastInNum(ast->getRhs()) or CanCastInNum(TypeOfIdentAst(ast->getRhs())));
				const auto RhsEval = evalNumExpr(ast->getRhs());
				if (ast->getOp() == "IdxAt")
				{
					const auto res = LhsEval.at(RhsEval);
					if (CanCastInNum(res))
					{
						return dynamic_cast<ast::NumberAst*>(res)->getVal();
					}
				}
			}
			assert(0 && "something wrong");
		}
		bool isLhsNumber = ast->getLhs()->getID() == ast::NumberID,
			 isRhsNumber = ast->getRhs()->getID() == ast::NumberID,
			 isLhsMonoExp = ast->getLhs()->getID() == ast::MonoExpID,
			 isRhsMonoExp = ast->getRhs()->getID() == ast::MonoExpID,
			 isLhsBinExp = ast->getLhs()->getID() == ast::BinaryExpID,
			 isRhsBinExp = ast->getRhs()->getID() == ast::BinaryExpID,
			 isLhsCall = ast->getLhs()->getID() == ast::CallID,
			 isRhsCall = ast->getRhs()->getID() == ast::CallID,
			 isLhsIdent = ast->getLhs()->getID() == ast::IdentID,
			 isRhsIdent = ast->getRhs()->getID() == ast::IdentID;

		
		if (isLhsCall)
		{
			auto lhseval = evalCallAst(dynamic_cast<ast::CallAst*>(ast->getLhs()));
			assert(CanCastInNum(lhseval));
		}
		if (isRhsCall)
		{
			auto rhseval = evalCallAst(dynamic_cast<ast::CallAst*>(ast->getRhs()));
			assert(CanCastInNum(rhseval));
		}

		const auto LhsEval =
			isLhsNumber ? evalNumberAst(dynamic_cast<ast::NumberAst*>(ast->getLhs())) : \
			isLhsMonoExp ? evalNumMonoExpAst(dynamic_cast<ast::MonoExpAst*>(ast->getLhs())) : \
			isLhsBinExp ? evalNumBinaryExpAst(dynamic_cast<ast::BinaryExpAst*>(ast->getLhs())) : \
			isLhsIdent ? evalNumExpr(dynamic_cast<ast::IdentAst*>(ast->getLhs())) : \
			isLhsCall ? evalNumExpr(evalCallAst(dynamic_cast<ast::CallAst*>(ast->getLhs()))) : \
			-1; // dummy
		const auto RhsEval =
			isRhsNumber ? evalNumberAst(dynamic_cast<ast::NumberAst*>(ast->getRhs())) : \
			isRhsMonoExp ? evalNumMonoExpAst(dynamic_cast<ast::MonoExpAst*>(ast->getRhs())) : \
			isRhsBinExp ? evalNumBinaryExpAst(dynamic_cast<ast::BinaryExpAst*>(ast->getRhs())) : \
			isRhsIdent ? evalNumExpr(dynamic_cast<ast::IdentAst*>(ast->getRhs())) : \
			isRhsCall ? evalNumExpr(evalCallAst(dynamic_cast<ast::CallAst*>(ast->getRhs()))) : \
			-1; // dummy
		if (ast->getOp() == "+") { return LhsEval + RhsEval; }
		else if (ast->getOp() == "-") { return LhsEval - RhsEval; }
		else if (ast->getOp() == "*") { return LhsEval * RhsEval; }
		else if (ast->getOp() == "/") { return LhsEval / RhsEval; }
		else if (ast->getOp() == "%") { return LhsEval % RhsEval; }
		else if (ast->getOp() == "==") { return static_cast<std::int_fast64_t>(LhsEval == RhsEval); }
		else if (ast->getOp() == "!=") { return static_cast<std::int_fast64_t>(LhsEval != RhsEval); }
		else if (ast->getOp() == "<") { return static_cast<std::int_fast64_t>(LhsEval < RhsEval); }
		else if (ast->getOp() == ">") { return static_cast<std::int_fast64_t>(LhsEval > RhsEval); }
		else if (ast->getOp() == "<=") { return static_cast<std::int_fast64_t>(LhsEval <= RhsEval); }
		else if (ast->getOp() == ">=") { return static_cast<std::int_fast64_t>(LhsEval >= RhsEval); }
		else if (ast->getOp() == "&&") { return static_cast<std::int_fast64_t>(LhsEval && RhsEval); }
		else if (ast->getOp() == "||") { return static_cast<std::int_fast64_t>(LhsEval || RhsEval); }
		else // otherwize
		{
			assert(0 && "no match");
		}
		return 0;
	}

	std::int_fast64_t evalNumMonoExpAst(ast::MonoExpAst* ast)
	{
		bool isLhsNumber = ast->getLhs()->getID() == ast::NumberID,
			 isLhsMonoExp = ast->getLhs()->getID() == ast::MonoExpID,
			 isLhsCall = ast->getLhs()->getID() == ast::CallID,
			 isLhsBinExp = ast->getLhs()->getID() == ast::BinaryExpID,
			 isLhsIdent = ast->getLhs()->getID() == ast::IdentID;
			 
		const auto LhsEval =
			isLhsNumber ? evalNumberAst(dynamic_cast<ast::NumberAst*>(ast->getLhs())) : \
			isLhsMonoExp ? evalNumMonoExpAst(dynamic_cast<ast::MonoExpAst*>(ast->getLhs())) : \
			isLhsBinExp ? evalNumBinaryExpAst(dynamic_cast<ast::BinaryExpAst*>(ast->getLhs())) : \
			isLhsIdent ? evalNumExpr(dynamic_cast<ast::IdentAst*>(ast->getLhs())) : \
			-1; // dummy
		if (ast->getOp() == "!") { return static_cast<std::int_fast64_t>(! LhsEval); }
		else if (ast->getOp() == "-") { return -LhsEval; }
		else if (ast->getOp() == "~") { return ~LhsEval; }
		else // otherwize
		{
			assert(0 && "no match");
		}
		return 0;
	}

	// ================
	//      tuple
	// ================

	std::vector<ast::BaseAst*> evalTplExpr(ast::BaseAst* ast)
	{
		std::vector<ast::BaseAst*> res{};
		std::vector<ast::BaseAst*> Ary{};

		if (ast->getID() == ast::IdentID)
		{
			const auto& name = dynamic_cast<ast::IdentAst*>(ast)->getIdent();
			bool flg = true;

			assert(valsSize >= 1);
			for (std::int_fast32_t i = valsSize - 1; i >= curLower; i--)
			{
				if (vals[i].find(name) != std::end(vals[i]))
				{
					if (CanCastInTpl(vals[i].at(name)))
					{
						Ary = evalTplExpr(vals[i].at(name));
						flg = false;
						break;
					}
					else
					{
						assert(0);
					}
				}
			}

			if (flg)
			{
				assert(0 && "unknown ident");
			}
		}
		else
		{
			Ary = dynamic_cast<ast::TupleAst*>(ast)->getAry();
		}

		assert(not Ary.empty());
		for (const auto& elm : Ary)
		{
			if (elm->getID() == ast::IdentID)
			{
				if (TypeOfIdentAst(elm) == ast::NumberID)
				{
					res.push_back(new ast::NumberAst(evalNumExpr(elm)));
				}
				else if (TypeOfIdentAst(elm) == ast::StringID)
				{
					res.push_back(new ast::StringAst(evalStrExpr(elm)));
				}
				else if (TypeOfIdentAst(elm) == ast::TupleID)
				{
					res.push_back(new ast::TupleAst(evalTplExpr(elm)));
				}
				else assert(0 && "unknown type");
			}
			else
			{
				if (CanCastInNum(elm))
				{
					res.push_back(new ast::NumberAst(evalNumExpr(elm)));
				}
				else if (CanCastInStr(elm))
				{
					res.push_back(new ast::StringAst(evalStrExpr(elm)));
				}
				else if (CanCastInTpl(elm))
				{
					res.push_back(new ast::TupleAst(evalTplExpr(elm)));
				}
				else assert(0 && "unknown type");
			}
		}

		return res;
	}
};

} // namespace eval
} // namespace Noh
