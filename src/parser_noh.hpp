#pragma once

#include <boost/fusion/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "ast_noh.hpp"

namespace qi = boost::spirit::qi;
namespace ph = boost::phoenix;
using qi::_1, qi::_val;

namespace Noh {
namespace parser {

template<class Iterator, class Skipper>
struct Calc : qi::grammar<Iterator, ast::ModuleAst*(), Skipper> {
	qi::rule <Iterator, std::string(), Skipper> Ident;
	qi::rule <Iterator, std::string()> _String;
	qi::rule <Iterator, ast::BaseAst*(), Skipper> StrExpr;
	qi::rule <Iterator, ast::FuncAst*(), Skipper> Func;
	qi::rule <Iterator, ast::CallAst*(), Skipper> Call;
	qi::rule <Iterator, ast::ModuleAst*(), Skipper> Module;
	qi::rule <Iterator, ast::BaseAst*(), Skipper> Stmt;
	qi::rule <Iterator, ast::BuiltinAst*(), Skipper> Builtin;
	qi::rule <Iterator, ast::AssignAst*(), Skipper> Assign;
	qi::rule <Iterator, ast::ReAssignAst*(), Skipper> ReAssign;
	qi::rule <Iterator, ast::IfStmtAst*(), Skipper> IfStmt;
	qi::rule <Iterator, ast::WhileStmtAst*(), Skipper> WhileStmt;
	qi::rule <Iterator, ast::RangeAst*(), Skipper> Range;
	qi::rule <Iterator, ast::ForStmtAst*(), Skipper> ForStmt;
	qi::rule <Iterator, std::vector<ast::BaseAst*>(), Skipper> Stmts;
	qi::rule <Iterator, ast::BaseAst*(), Skipper> Factor;
	qi::rule <Iterator, ast::BaseAst*(), Skipper> NumExpr, E1, E2, E3, E4, E5;
	qi::rule <Iterator, std::vector<ast::BaseAst*>(), Skipper> _TupleExpr;
	qi::rule <Iterator, ast::BaseAst*(), Skipper> TupleExpr;
	qi::rule <Iterator, ast::BaseAst*(), Skipper> Expr;

	Calc() : Calc::base_type(Module)
	{
		Module = qi::eps[_val = ph::new_<ast::ModuleAst>()] >> *(Func[ph::push_back(ph::at_c<0>(*_val), _1)]);

		Ident = qi::lexeme[(qi::alpha | qi::char_('_'))[_val = _1] >> *((qi::alnum | qi::char_('_'))[_val += _1])];

		_String = qi::char_('\"')[_val = ""]
			>> *((qi::char_('\\')[_val += '\\'] >> qi::char_[_val += _1])
				| (qi::char_ - qi::char_('\"'))[_val += _1])
			>> qi::char_('\"');
		
		StrExpr = _String[_val = ph::new_<ast::StringAst>(_1)];
		
		_TupleExpr = 
			('[' >> -(Expr[ph::push_back(_val, _1)]
				>> *(',' >> Expr[ph::push_back(_val, _1)])) >> ']');
		TupleExpr = _TupleExpr[_val = ph::new_<ast::TupleAst>(_1)];

		Range = Expr[_val = ph::new_<ast::RangeAst>(), ph::at_c<0>(*_val) = _1] >> ".." >> Expr[ph::at_c<1>(*_val) = _1];

		Func = "fn" >> Ident[_val = ph::new_<ast::FuncAst>(_1)]
			>> '(' >> -(Ident[ph::push_back(ph::at_c<1>(*_val), _1)] >> *(',' >> Ident[ph::push_back(ph::at_c<1>(*_val), _1)])) >> ')'
			>> '{' >> *Stmt[ph::push_back(ph::at_c<2>(*_val), _1)] >> '}';

		Call = Ident[_val = ph::new_<ast::CallAst>(_1)]
			>> '(' >> -(Expr[ph::push_back(ph::at_c<1>(*_val), _1)]
			>> *(',' >> Expr[ph::push_back(ph::at_c<1>(*_val), _1)])) >> ')';

		Stmt = Builtin | IfStmt | WhileStmt | ForStmt | Assign | ReAssign | (Call >> ';');

		Builtin =
			 (("break" >> qi::eps[_val = ph::new_<ast::BuiltinAst>("break")])
			| ("continue" >> qi::eps[_val = ph::new_<ast::BuiltinAst>("continue")])
			| ("exit" >> qi::eps[_val = ph::new_<ast::BuiltinAst>("exit")])
			| ("return" >> qi::eps[_val = ph::new_<ast::BuiltinAst>("return")]
				>> Expr[ph::push_back(ph::at_c<1>(*_val), _1)])
			| ("print" >> qi::char_('(') >> qi::eps[_val = ph::new_<ast::BuiltinAst>("print")]
				>> Expr[ph::push_back(ph::at_c<1>(*_val), _1)]
				>> *(',' >> Expr[ph::push_back(ph::at_c<1>(*_val), _1)])
				>> qi::char_(')'))
			| ("scanNum" >> qi::char_('(') >> Ident[_val = ph::new_<ast::BuiltinAst>("scanNum"), ph::push_back(ph::at_c<1>(*_val), ph::new_<ast::IdentAst>(_1))] >> qi::char_(')'))
			| ("scanStr" >> qi::char_('(') >> Ident[_val = ph::new_<ast::BuiltinAst>("scanStr"), ph::push_back(ph::at_c<1>(*_val), ph::new_<ast::IdentAst>(_1))] >> qi::char_(')')))
				>> ';';

		Assign = "var" >> Ident[_val = ph::new_<ast::AssignAst>(_1)] >> '='
			>> (Expr[ph::at_c<1>(*_val) = _1] | Ident[ph::at_c<1>(*_val) = ph::new_<ast::IdentAst>(_1)]) >> ';';
		
		ReAssign = Ident[_val = ph::new_<ast::ReAssignAst>(_1)] >> '='
			>> (Expr[ph::at_c<1>(*_val) = _1] | Ident[ph::at_c<1>(*_val) = ph::new_<ast::IdentAst>(_1)]) >> ';';

		IfStmt = "if" >> Expr[_val = ph::new_<ast::IfStmtAst>(), ph::at_c<0>(*_val) = _1]
			>> '{' >> *Stmt[ph::push_back(ph::at_c<1>(*_val), _1)]
			>> -(qi::char_('}') >> "else" >> '{' >> *Stmt[ph::push_back(ph::at_c<2>(*_val), _1)]) >> '}';

		WhileStmt = "while" >> Expr[_val = ph::new_<ast::WhileStmtAst>(), ph::at_c<0>(*_val) = _1]
			>> "{" >> *Stmt[ph::push_back(ph::at_c<1>(*_val), _1)] >> '}';
		
		ForStmt = "for" >> Ident[_val = ph::new_<ast::ForStmtAst>(), ph::at_c<0>(*_val) = _1] >> "in" >> Range[ph::at_c<1>(*_val) = _1]
			>> '{' >> *Stmt[ph::push_back(ph::at_c<2>(*_val), _1)] >> '}';

		Factor = qi::int_[_val = ph::new_<ast::NumberAst>(_1)]
			| Call[_val = _1]
			| Ident[_val = ph::new_<ast::IdentAst>(_1)]
			| '(' >> NumExpr[_val = _1] >> ')';
		E1 = Factor[_val = _1]
			| ('!' >> Factor[_val = ph::new_<ast::MonoExpAst>("!", _1)])
			| ('-' >> Factor[_val = ph::new_<ast::MonoExpAst>("-", _1)])
			| ('~' >> Factor[_val = ph::new_<ast::MonoExpAst>("~", _1)]);
		E2 = E1[_val = _1] >>
			*(('*' >> E1[_val = ph::new_<ast::BinaryExpAst>("*", _val, _1)])
			| ('/' >> E1[_val = ph::new_<ast::BinaryExpAst>("/", _val, _1)])
			| ('%' >> E1[_val = ph::new_<ast::BinaryExpAst>("%", _val, _1)]));
		E3 = E2[_val = _1] >>
			*(('+' >> E2[_val = ph::new_<ast::BinaryExpAst>("+", _val, _1)])
			| ('-' >> E2[_val = ph::new_<ast::BinaryExpAst>("-", _val, _1)]));
		E4 = E3[_val = _1] >>
			*(("==" >> E3[_val = ph::new_<ast::BinaryExpAst>("==", _val, _1)])
			| ("!=" >> E3[_val = ph::new_<ast::BinaryExpAst>("!=", _val, _1)])
			| ('<' >> E3[_val = ph::new_<ast::BinaryExpAst>("<", _val, _1)])
			| ('>' >> E3[_val = ph::new_<ast::BinaryExpAst>(">", _val, _1)])
			| ("<=" >> E3[_val = ph::new_<ast::BinaryExpAst>("<=", _val, _1)])
			| (">=" >> E3[_val = ph::new_<ast::BinaryExpAst>(">=", _val, _1)]));
		E5 = E4[_val = _1] >>
			*(("&&" >> E4[_val = ph::new_<ast::BinaryExpAst>("&&", _val, _1)])
			| ("||" >> E4[_val = ph::new_<ast::BinaryExpAst>("||", _val, _1)]));
		NumExpr = E5;

		Expr =
			NumExpr[_val = _1]
			| ((TupleExpr[_val = _1] | Ident[_val = ph::new_<ast::IdentAst>(_1)]) >>
				'(' >> Expr[_val = ph::new_<ast::BinaryExpAst>("IdxAt", _val, _1)] >> ')')
			| StrExpr[_val = _1]
			| TupleExpr[_val = _1];
	}
};

} // namespace parser
} // namespace Noh