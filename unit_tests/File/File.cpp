#include <neolib/neolib.hpp>
#include <neolib/file/lexer.hpp>

enum class token
{
    Program,
    Whitespace,
    FunctionDefinition,
    FunctionPrototype,
    FunctionBody,
    FunctionReturnType,
    FunctionName,
    FunctionParameterList,
    FunctionParameter,
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

enable_neolib_lexer(token)

char const* source = R"test(
    void foo()
    {
        a := 42.0 * 1.0;
    }
)test";

int main(int argc, char** argv)
{   
    neolib::lexer_rule<token> lexerRules[] =
    {
        ( token::Program >> repeat(token::FunctionDefinition) ),
        ( token::FunctionDefinition >> token::FunctionPrototype , token::FunctionBody ),
        ( token::FunctionPrototype >> token::FunctionReturnType , token::FunctionName , 
            '(' , optional(token::FunctionParameterList) , ')' ),
        ( token::FunctionParameterList >> 
            token::FunctionParameter , optional(repeat(sequence(',' , token::FunctionParameter))) ),
        ( token::FunctionBody >> repeat(token::Expression) ),
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

        ( token::Whitespace >> discard(optional(repeat(' '_ | '\r' | '\n' | '\t'))) ),
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

    neolib::lexer<token> parser;
    auto result = parser.parse(source);
}

