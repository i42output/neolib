#include <neolib/neolib.hpp>
#include <neolib/core/i_enum.hpp>
#include <neolib/file/lexer.hpp>

namespace lexer_test
{
    enum class token
    {
        Program,
        Whitespace,
        Identifier,
        FunctionDefinition,
        FunctionPrototype,
        FunctionBody,
        FunctionReturnType,
        FunctionName,
        FunctionParameterList,
        FunctionParameter,
        Type,
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
        Name,
        Assign,
        Equal
    };
}

declare_tokens(lexer_test::token)
declare_token(lexer_test::token, Program)
declare_token(lexer_test::token, Whitespace)
declare_token(lexer_test::token, Identifier)
declare_token(lexer_test::token, FunctionDefinition)
declare_token(lexer_test::token, FunctionPrototype)
declare_token(lexer_test::token, FunctionBody)
declare_token(lexer_test::token, FunctionReturnType)
declare_token(lexer_test::token, FunctionName)
declare_token(lexer_test::token, FunctionParameterList)
declare_token(lexer_test::token, FunctionParameter)
declare_token(lexer_test::token, Type)
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
declare_token(lexer_test::token, Name)
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
        a := 42.0 * 1.0;
    }
)test";

int main(int argc, char** argv)
{
    using namespace lexer_test;

    neolib::lexer_rule<token> lexerRules[] =
    {
        ( token::Program >> repeat(token::FunctionDefinition) ),
        ( token::FunctionDefinition >> token::FunctionPrototype , token::FunctionBody ),
        ( token::FunctionPrototype >> token::FunctionReturnType , token::FunctionName , 
            '(' , optional(token::FunctionParameterList) , ')' ),
        ( token::FunctionReturnType >> token::Type ),
        ( token::FunctionParameterList >> 
            token::FunctionParameter , optional(repeat(sequence(',' , token::FunctionParameter))) ),
        ( token::FunctionBody >> repeat(token::Expression) ),
        ( token::Type >> token::Identifier ),
        ( token::Identifier >> sequence(repeat(range('A', 'Z') | range('a', 'z')) , repeat(range('A', 'Z') | range('a', 'z') | range('0', '9'))) ),
        ( token::Expression >> token::Expression , choice(token::Add | token::Subtract) , token::Term ),
        ( token::Expression >> token::Term ),
        ( token::Term >> token::Term , choice(token::Divide | token::Multiply) , token::Primary ),
        ( token::Term >> token::Primary ),
        ( token::Primary >> token::Number ),
        ( token::Primary >> token::Name ),
        ( token::Primary >> token::Name , token::Assign , token::Expression ),
        ( token::Primary >> token::Negate , token::Primary ),
        ( token::Primary >> '(' , token::Expression , ')' ),
        ( token::Add >> '+' ),
        ( token::Subtract >> '-' ),
        ( token::Multiply >> '*' ),
        ( token::Divide >> '/' ),
        ( token::Negate >> '-' ),
        ( token::Assign >> ":=" ),
        ( token::Equal >> '=' ),
        ( token::Number >> repeat(token::Digit) , token::Decimal , repeat(token::Digit) ),
        ( token::Digit >> range('0' , '9') ),
        ( token::Decimal >> '.' ),

        // whitespace handling...

        ( token::Whitespace >> discard(optional(' '_ | '\r' | '\n' | '\t')) ),
        ( token::Program >> token::Whitespace , token::Program , token::Whitespace ),
        ( token::FunctionDefinition >> token::Whitespace , token::FunctionDefinition , token::Whitespace ),
        ( token::FunctionPrototype >> token::Whitespace , token::FunctionPrototype , token::Whitespace ),
        ( token::FunctionBody >> token::Whitespace , token::FunctionBody , token::Whitespace ),
        ( token::FunctionReturnType >> token::Whitespace , token::FunctionReturnType , token::Whitespace ),
        ( token::FunctionName >> token::Whitespace , token::FunctionName , token::Whitespace ),
        ( token::FunctionParameter >> token::Whitespace , token::FunctionParameter , token::Whitespace ),
        ( token::Expression >> token::Whitespace , token::Expression , token::Whitespace ),
        ( token::Term >> token::Whitespace , token::Term , token::Whitespace ),
        ( token::Primary >> token::Whitespace , token::Primary , token::Whitespace )
    };

    neolib::lexer<token> parser{ lexerRules };
    parser.set_debug_output(std::cerr);
    auto result = parser.parse(token::Program, source);
}

