#include <neolib/neolib.hpp>
#include <neolib/core/i_enum.hpp>
#include <neolib/file/lexer.hpp>

namespace lexer_test
{
    enum class token
    {
        Program,
        Whitespace,
        Eof,
        Identifier,
        FunctionDefinition,
        FunctionPrototype,
        FunctionBody,
        FunctionReturnType,
        FunctionName,
        FunctionParameterList,
        FunctionParameter,
        OpenScope,
        CloseScope,
        Type,
        Statement,
        EndStatement,
        Expression,
        Term,
        Primary,
        Add,
        Subtract,
        Multiply,
        Divide,
        Negate,
        Number,
        Digit,
        Decimal,
        Variable,
        Assign,
        Equal,
    };
}

declare_tokens(lexer_test::token)
declare_token(lexer_test::token, Program)
declare_token(lexer_test::token, Whitespace)
declare_token(lexer_test::token, Eof)
declare_token(lexer_test::token, Identifier)
declare_token(lexer_test::token, FunctionDefinition)
declare_token(lexer_test::token, FunctionPrototype)
declare_token(lexer_test::token, FunctionBody)
declare_token(lexer_test::token, FunctionReturnType)
declare_token(lexer_test::token, FunctionName)
declare_token(lexer_test::token, FunctionParameterList)
declare_token(lexer_test::token, FunctionParameter)
declare_token(lexer_test::token, OpenScope)
declare_token(lexer_test::token, CloseScope)
declare_token(lexer_test::token, Type)
declare_token(lexer_test::token, Statement)
declare_token(lexer_test::token, EndStatement)
declare_token(lexer_test::token, Expression)
declare_token(lexer_test::token, Term)
declare_token(lexer_test::token, Primary)
declare_token(lexer_test::token, Add)
declare_token(lexer_test::token, Subtract)
declare_token(lexer_test::token, Multiply)
declare_token(lexer_test::token, Divide)
declare_token(lexer_test::token, Negate)
declare_token(lexer_test::token, Number)
declare_token(lexer_test::token, Digit)
declare_token(lexer_test::token, Decimal)
declare_token(lexer_test::token, Variable)
declare_token(lexer_test::token, Assign)
declare_token(lexer_test::token, Equal)
end_declare_tokens(lexer_test::token);

namespace lexer_test
{
    enable_neolib_lexer(token)
}

std::string_view const source = R"test(
    void foo()
    {
        x := 7 + 42.0 * 1.0;
    }
)test";

int main(int argc, char** argv)
{
    using namespace lexer_test;

    neolib::lexer_rule<token> lexerRules[] =
    {
        ( token::Program >> repeat(token::FunctionDefinition) , discard(token::Eof) ),
        ( token::FunctionDefinition >> token::FunctionPrototype , token::FunctionBody ),
        ( token::FunctionPrototype >> token::FunctionReturnType , token::FunctionName , 
            discard('(') , optional(token::FunctionParameterList) , discard(')') ),
        ( token::FunctionReturnType >> token::Type ),
        ( token::FunctionName >> token::Identifier ),
        ( token::FunctionParameterList >> 
            token::FunctionParameter , optional(repeat(sequence(',' , token::FunctionParameter))) ),
        ( token::FunctionBody >> discard(token::OpenScope) , optional(repeat(token::Statement)) , discard(token::CloseScope) ),
        ( token::Type >> token::Identifier ),
        ( token::Identifier >> sequence(repeat(range('A', 'Z') | range('a', 'z')) , optional(repeat(range('A', 'Z') | range('a', 'z') | range('0', '9')))) ),
        ( token::OpenScope >> '{' ),
        ( token::CloseScope >> '}' ),
        ( token::Statement >> token::Expression , discard(token::EndStatement) ),
        ( token::EndStatement >> ';' ),
        ( token::Expression >> token::Expression , choice(token::Add | token::Subtract) , token::Term ),
        ( token::Expression >> token::Term ),
        ( token::Term >> token::Term , choice(token::Divide | token::Multiply) , token::Primary ),
        ( token::Term >> token::Primary ),
        ( token::Primary >> token::Variable , token::Assign , token::Expression ),
        ( token::Primary >> token::Negate , token::Primary ),
        ( token::Primary >> token::Number ),
        ( token::Primary >> token::Variable ),
        ( token::Primary >> '(' , token::Expression , ')' ),
        ( token::Add >> '+' ),
        ( token::Subtract >> '-' ),
        ( token::Multiply >> '*' ),
        ( token::Divide >> '/' ),
        ( token::Negate >> '-' ),
        ( token::Assign >> ":=" ),
        ( token::Equal >> '=' ),
        ( token::Number >> repeat(token::Digit) , optional(token::Decimal) , optional(repeat(token::Digit)) ),
        ( token::Digit >> range('0' , '9') ),
        ( token::Decimal >> '.' ),
        ( token::Variable >> token::Identifier ),

        // whitespace handling...

        ( token::Eof >> "" ),
        ( token::Whitespace >> discard(optional(' '_ | '\r' | '\n' | '\t' ))),
        ( token::Program >> discard(token::Whitespace) , token::Program , discard(token::Whitespace) ),
        ( token::FunctionDefinition >> discard(token::Whitespace) , token::FunctionDefinition , discard(token::Whitespace) ),
        ( token::FunctionPrototype >> discard(token::Whitespace) , token::FunctionPrototype , discard(token::Whitespace) ),
        ( token::FunctionBody >> discard(token::Whitespace) , token::FunctionBody , discard(token::Whitespace) ),
        ( token::FunctionReturnType >> discard(token::Whitespace) , token::FunctionReturnType , discard(token::Whitespace) ),
        ( token::FunctionName >> discard(token::Whitespace) , token::FunctionName , discard(token::Whitespace) ),
        ( token::FunctionParameter >> discard(token::Whitespace) , token::FunctionParameter , discard(token::Whitespace) ),
        ( token::OpenScope >> discard(sequence(token::Whitespace , token::OpenScope , token::Whitespace)) ),
        ( token::CloseScope >> discard(sequence(token::Whitespace , token::CloseScope , token::Whitespace)) ),
        ( token::Statement >> discard(token::Whitespace) , token::Statement , discard(token::Whitespace) ),
        ( token::EndStatement >> discard(sequence(token::Whitespace , token::EndStatement , token::Whitespace)) ),
        ( token::Expression >> discard(token::Whitespace) , token::Expression , discard(token::Whitespace) ),
        ( token::Variable >> discard(token::Whitespace) , token::Variable , discard(token::Whitespace) ),
        ( token::Identifier >> discard(token::Whitespace) , token::Identifier , discard(token::Whitespace) ),
        ( token::Number >> discard(token::Whitespace) , token::Number, discard(token::Whitespace) ),
        ( token::Assign >> discard(token::Whitespace) , token::Assign, discard(token::Whitespace) ),
        ( token::Equal >> discard(token::Whitespace) , token::Equal, discard(token::Whitespace) ),
        ( token::Add >> discard(token::Whitespace) , token::Add, discard(token::Whitespace) ),
        ( token::Subtract >> discard(token::Whitespace) , token::Subtract, discard(token::Whitespace) ),
        ( token::Multiply >> discard(token::Whitespace) , token::Multiply, discard(token::Whitespace) ),
        ( token::Divide >> discard(token::Whitespace) , token::Divide, discard(token::Whitespace) ),
        ( token::Negate >> discard(token::Whitespace) , token::Negate, discard(token::Whitespace) ),
        ( token::Term >> discard(token::Whitespace) , token::Term , discard(token::Whitespace) ),
        ( token::Primary >> discard(token::Whitespace) , token::Primary , discard(token::Whitespace) )
    };

    neolib::lexer<token> parser{ lexerRules };
    parser.set_debug_output(std::cerr);
    auto result = parser.parse(token::Program, source);
}

