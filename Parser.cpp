///////////////////////////////////////////////////////////////////////////////
//
//  Parser.cpp
//
//  Copyright © Pete Isensee (PKIsensee@msn.com).
//  All rights reserved worldwide.
//
//  Permission to copy, modify, reproduce or redistribute this source code is
//  granted provided the above copyright notice is retained in the resulting 
//  source code.
// 
//  This software is provided "as is" and without any express or implied
//  warranties.
// 
//  Derived from Crafting Interpreters by Robert Nystrom
//  https://craftinginterpreters.com/
//
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iostream>
#include <ranges>

#include "CompilerError.h"
#include "Lexer.h"
#include "Parser.h"
#include "Util.h"

using namespace PKIsensee;
namespace ranges = std::ranges;

static constexpr size_t kMaxFunctionArguments = 1000; // this should be enough :)

///////////////////////////////////////////////////////////////////////////////
//
// Parse the incoming stream into tokens

void Parser::Parse( std::string_view source )
{
  std::cout << "Parse: " << source << '\n';
  tokens_.resize( 0 );
  Lexer lexer( source );
  for( ;;)
  {
    Token token = lexer.GetNextToken();
    tokens_.push_back( token );
    if( token.GetType() == TokenType::EndOfFile )
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// Generate list of statements

StmtList Parser::GetStatements()
{
  StmtList statements; // TODO member data?
  currToken_ = 0;
  try
  {
    while( Peek().GetType() != TokenType::EndOfFile )
      statements.push_back( GetDecl() );
    return statements;
  }
  catch( CompilerError& )
  {
    PK_VALID( false ); // TODO
    return {};
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// Generate AST

std::expected<AbstractSyntaxTree, CompilerError> Parser::GetAST()
{
  currToken_ = 0;
  try
  {
    AbstractSyntaxTree ast{ GetExpr() };
    return ast;
  }
  catch( CompilerError& err )
  {
    return std::unexpected( err );
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// True if all tokens are valid

bool Parser::AllTokensValid() const
{
  return ranges::all_of( tokens_, []( const auto& token )
    {
      return token.GetType() != TokenType::Invalid;
    } );
}

///////////////////////////////////////////////////////////////////////////////
//
// If the token at the current position matches the incoming type, advance
// to the next token

Token Parser::Consume( TokenType tokenType, std::string_view errMsg ) // private
{
  if( IsTokenMatch( tokenType ) )
    return Advance();

  throw CompilerError{ errMsg, Peek() };
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the expression at the highest level of precedence
// 
// Grammar: literal | identifier | '(' parens-expr ')'

ExprPtr Parser::GetPrimaryExpr()
{
  if (IsMatch( TokenType::Number, TokenType::String, 
               TokenType::True, TokenType::False ) )
    return std::make_unique<LiteralExpr>( GetPrevToken() );

  if( IsMatch( TokenType::Identifier ) ) {
    return std::make_unique<VarExpr>( GetPrevToken() );
  }

  if( IsMatch( TokenType::OpenParen ) )
  {
    ExprPtr expr = GetExpr();
    Consume( TokenType::CloseParen, "Expected ')' after expression" );
    return std::make_unique<ParensExpr>( std::move(expr) );
  }

  throw CompilerError{ "Expected an expression", Peek() };
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a function call
//
// Grammar: primary-expr '(' [arguments] ')'

ExprPtr Parser::GetFuncCallExpr()
{
  ExprPtr expr = GetPrimaryExpr();
  while( IsMatch( TokenType::OpenParen ) )
    expr = FinishFuncCallExpr( std::move(expr) );
  return expr;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a function call, phase two

ExprPtr Parser::FinishFuncCallExpr( ExprPtr fnName )
{
  ExprList arguments;
  if( !IsTokenMatch( TokenType::CloseParen ) )
  {
    do {
      if( arguments.size() > kMaxFunctionArguments )
        throw CompilerError{ "Too many arguments", GetPrevToken() };
      arguments.push_back( GetExpr() );
    } while( IsMatch( TokenType::Comma ) );
  }
  [[maybe_unused]] Token paren = Consume( TokenType::CloseParen, 
                                          "Expected ')' after function args" );
  return std::make_unique<FuncExpr>( std::move(fnName), std::move(arguments) );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a unary expression
// 
// Grammar: ('!' | '-') unary-expr | fn-expr

ExprPtr Parser::GetUnaryExpr()
{
  if( IsMatch( TokenType::Not, TokenType::Minus ) )
  {
    Token unaryOp = GetPrevToken();
    ExprPtr unaryExpr = GetUnaryExpr();
    return std::make_unique<UnaryExpr>( unaryOp, std::move(unaryExpr) );
  }
  return GetFuncCallExpr();
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a multiply or division expression
// 
// Grammar: unary-expr ('*' | '/') unary-expr

ExprPtr Parser::GetMultiplicationExpr()
{
  auto exprFn = std::bind( &Parser::GetUnaryExpr, this );
  return GetBinaryExpr( exprFn, TokenType::Multiply, TokenType::Divide );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract an addition or subtraction expression
// 
// Grammar: multiply-expr ('+' | '-') multiply-expr

ExprPtr Parser::GetAdditionExpr()
{
  auto exprFn = std::bind( &Parser::GetMultiplicationExpr, this );
  return GetBinaryExpr( exprFn, TokenType::Plus, TokenType::Minus );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a comparison expression
// 
// Grammar: add-expr ('<' | '<=' | '>' | '>=' ) add-expr

ExprPtr Parser::GetComparisonExpr()
{
  ExprPtr lhs = GetAdditionExpr();
  while( IsMatch( TokenType::GreaterThan,
                  TokenType::GreaterThanEqual,
                  TokenType::LessThan,
                  TokenType::LessThanEqual ) )
  {
    Token compOp = GetPrevToken();
    ExprPtr rhs = GetAdditionExpr();
    lhs = std::make_unique<BinaryExpr>( std::move(lhs), compOp, std::move(rhs) );
  }
  return lhs;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract an equal or not-equal expression
// 
// Grammar: comp-expr ('==' | '!=' ) comp-expr

ExprPtr Parser::GetEqualityExpr()
{
  ExprPtr lhs = GetComparisonExpr();
  while( IsMatch( TokenType::NotEqual, TokenType::IsEqual ) )
  {
    Token compOp = GetPrevToken();
    ExprPtr rhs = GetComparisonExpr();
    lhs = std::make_unique<BinaryExpr>( std::move(lhs), compOp, std::move(rhs) );
  }
  return lhs;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract an AND expression
// 
// Grammar: eq-expr 'and' eq-expr

ExprPtr Parser::GetAndExpr()
{
  ExprPtr lhs = GetEqualityExpr();
  while( IsMatch( TokenType::And ) ) // TODO IsMatch -> IsTokenMatch
  {
    Token andOp = GetPrevToken();
    ExprPtr rhs = GetEqualityExpr();
    lhs = std::make_unique<LogicalExpr>( std::move(lhs), andOp, std::move(rhs) );
  }
  return lhs;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract an OR expression
// 
// Grammar: and-expr 'or' and-expr

ExprPtr Parser::GetOrExpr()
{
  ExprPtr lhs = GetAndExpr();
  while( IsMatch( TokenType::Or ) )
  {
    Token orOp = GetPrevToken();
    ExprPtr rhs = GetAndExpr();
    lhs = std::make_unique<LogicalExpr>( std::move( lhs ), orOp, std::move( rhs ) );
  }
  return lhs;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract the assignment expression
// 
// Grammar: identifier '=' assign-expr | or-expr

ExprPtr Parser::GetAssignExpr()
{
  ExprPtr lhs = GetOrExpr();
  if( IsMatch( TokenType::Assign ) )
  {
    Token assignOp = GetPrevToken();
    ExprPtr rhs = GetAssignExpr();

    // Parse the left hand side as if it were an expression, but convert
    // to an assignment if we determine it's actually a variable
    if( auto* varExpr = dynamic_cast<VarExpr*>( lhs.get() ); varExpr )
      return std::make_unique<AssignExpr>( varExpr->GetVariable(), std::move(rhs) );

    throw CompilerError( "Invalid assignment target", assignOp );
  }
  return lhs;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract an expression at any precedence level. Since assignment has the lowest
// precedence, it covers everything else.
// 
// Grammar: assign-expr

ExprPtr Parser::GetExpr()
{
  return GetAssignExpr();
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a statement.
// 
// Grammar: expression ';'

StmtPtr Parser::GetExprStmt()
{
  ExprPtr expr = GetExpr();
  Consume( TokenType::EndStatement, "Expected ';' after expression" );
  return std::make_unique<ExprStmt>( std::move(expr) );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a block, which is a (possibly empty) series of statements surrounded
// by curly braces. A block itself is a statement.
// 
// Grammar: '{' declarations '}'

StmtList Parser::GetBlock()
{
  StmtList statements;
  while( !IsTokenMatch( TokenType::CloseBrace ) ) // TODO && ( Peek().GetType() != TokenType::EndOfFile )
    statements.push_back( GetDecl() );

  Consume( TokenType::CloseBrace, "Expected '}' after block" );
  return statements;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a while statement
// 
// Grammar: 'while' '(' expr ')' statement

StmtPtr Parser::GetWhileStmt()
{
  Consume( TokenType::OpenParen, "Expected '(' after 'while'" );
  ExprPtr condition = GetExpr();
  Consume( TokenType::CloseParen, "Expected ')' after 'while'" );
  return std::make_unique<WhileStmt>( std::move(condition), GetStmt() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a return statement. The expression is optional and may be empty.
// 
// Grammar: 'return' [expr] ';'

StmtPtr Parser::GetReturnStmt()
{
  ExprPtr expr;
  if( !IsTokenMatch( TokenType::EndStatement ) )
    expr = GetExpr();
  Consume( TokenType::EndStatement, "Expected ';' after return statement" );
  return std::make_unique<ReturnStmt>( std::move(expr) );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a print statement
// 
// Grammar: 'print' expr ';'

StmtPtr Parser::GetPrintStmt()
{
  ExprPtr expr = GetExpr();
  Consume( TokenType::EndStatement, "Expected ';' after print statement" );
  return std::make_unique<PrintStmt>( std::move(expr) );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract an if statement. The else clause is optional.
// 
// Grammar: 'if' '(' expr ')' statement [ 'else' statement ]

StmtPtr Parser::GetIfStmt()
{
  Consume( TokenType::OpenParen, "Expected '(' after 'if' statement" );
  ExprPtr condition = GetExpr();
  Consume( TokenType::CloseParen, "Expected ')' after 'if' statement" );
  StmtPtr thenBranch = GetStmt();
  StmtPtr elseBranch;
  if( IsMatch( TokenType::Else ) )
    elseBranch = GetStmt();
  return std::make_unique<IfStmt>( std::move(condition), 
                                   std::move(thenBranch), std::move(elseBranch) );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a for statement and convert to an equivalent while loop
// 
// Grammar: 'for' '(' var-decl | init-expr ';' condition-expr ';' update-expr ')' statement

StmtPtr Parser::GetForStmt()
{
  Consume( TokenType::OpenParen, "Expected '(' after 'for' keyword" );
  StmtPtr initExpr;
  if( IsMatch( TokenType::EndStatement ) )
    initExpr = nullptr;
  else if( IsMatch( TokenType::Str, TokenType::Int, TokenType::Char, TokenType::Bool ) )
    initExpr = GetVarDecl();
  else
    initExpr = GetExprStmt();

  // TODO make it an error to have an empty condition?
  ExprPtr condition;
  if( !IsTokenMatch( TokenType::EndStatement ) )
    condition = GetExpr();
  Consume( TokenType::EndStatement, "Expected ';' after loop condition" );

  ExprPtr updateExpr;
  if( !IsTokenMatch( TokenType::CloseParen ) )
    updateExpr = GetExpr();
  Consume( TokenType::CloseParen, "Expected ')' after for clauses" );

  // Convert to while loop
  // for(<init>; <cond>; <update>) <body> is equivalent to:
  // <init>; while (<cond>) { <body>; <update>; }

  StmtPtr body = GetStmt();
  if( updateExpr )
  {
    // Merge <body> and <update>
    StmtList statements;
    statements.push_back( std::move(body) );
    statements.push_back( std::make_unique<ExprStmt>( std::move(updateExpr) ) );
    body = std::make_unique<BlockStmt>( std::move(statements) );
  }

  // Turn into while loop
  if( !condition )
    condition = std::make_unique<LiteralExpr>( Value{ true } );
  body = std::make_unique<WhileStmt>( std::move(condition), std::move(body) );

  // Put <init> first
  if( initExpr )
  {
    StmtList statements;
    statements.push_back( std::move(initExpr) );
    statements.push_back( std::move(body) );
    body = std::make_unique<BlockStmt>( std::move(statements) );
  }
  return body;
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a statement
// 
// Grammar: expr-statement | for-statement | if-statement | print-statement |
//          return-statement | while-statement | block

StmtPtr Parser::GetStmt()
{
  if( IsMatch( TokenType::For ) )
    return GetForStmt();

  if( IsMatch( TokenType::If ) )
    return GetIfStmt();

  if( IsMatch( TokenType::Print ) )
    return GetPrintStmt();

  if( IsMatch( TokenType::Return ) )
    return GetReturnStmt();

  if( IsMatch( TokenType::While ) )
    return GetWhileStmt();

  if( IsMatch( TokenType::OpenBrace ) )
    return std::make_unique<BlockStmt>( GetBlock() );

  return GetExprStmt();
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a function
// 
// Grammar: func-name '(' [params] ')' block

StmtPtr Parser::GetFunc()
{
  Token fnName = Consume( TokenType::Identifier, "Expected function name" );
  Consume( TokenType::OpenParen, "Expected '(' after function name" );
  ParamList parameters;
  if( !IsTokenMatch( TokenType::CloseParen ) )
  {
    do {
      if( parameters.size() > kMaxFunctionArguments )
        throw CompilerError{ "Too many arguments", fnName };

      Param param;
      if( IsMatch( TokenType::Str, TokenType::Int, TokenType::Char, TokenType::Bool ) )
        param.first = GetPrevToken();
      else
        throw CompilerError{ "Expected parameter type", Peek() };
      param.second = Consume( TokenType::Identifier, "Expected parameter name" );
      parameters.push_back( param );
    } while( IsMatch( TokenType::Comma ) );
  }
  Consume( TokenType::CloseParen, "Expected ')' after parameters" );
  Consume( TokenType::OpenBrace, "Expected '{' after function" );
  StmtList body = GetBlock();
  return std::make_unique<FuncStmt>( fnName, parameters, std::move(body) );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a variable declaration
// 
// Grammar: identifier '=' init-expr ';'

StmtPtr Parser::GetVarDecl() // pass in str/int/char/bool info
{
  Token variableType = GetPrevToken();
  Token variableName = Consume( TokenType::Identifier, "Expected a variable name" );
  Consume( TokenType::Assign, "Expected '=' variable initialization" );
  ExprPtr initializer = GetExpr();
  Consume( TokenType::EndStatement, "Expected ';' after variable initialization" );
  return std::make_unique<VarDeclStmt>( variableType, variableName, std::move( initializer ) );
}

///////////////////////////////////////////////////////////////////////////////
//
// Extract a declaration, which is a function, variable declaration or statement
// 
// Grammar: var-decl | statement

StmtPtr Parser::GetDecl()
{
  if( IsMatch( TokenType::Function ) )
    return GetFunc();
  if( IsMatch( TokenType::Str, TokenType::Int, TokenType::Char, TokenType::Bool ) )
    return GetVarDecl();
  return GetStmt();
}

///////////////////////////////////////////////////////////////////////////////
//
// Stream token data as text for debugging

std::ostream& PKIsensee::operator<<( std::ostream& out, const Parser& parser )
{
  for( const auto& token : parser.tokens_ )
    out << token << '\n';
  return out;
}

///////////////////////////////////////////////////////////////////////////////
