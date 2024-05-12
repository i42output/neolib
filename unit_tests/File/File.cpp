#include <neolib/neolib.hpp>
#include <neolib/file/lexer.hpp>

enum class token
{
    Document,
    Whitespace,
    FunctionDefinition,
    FunctionPrototype,
    FunctionBody,
    FunctionReturnType,
    FunctionName,
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
    Name,
    Assign,
    Equal
};

enable_neolib_lexer(token)

char const* source = R"test(
    void foo()
    {
    }
)test";

int main(int argc, char** argv)
{   
    neolib::lexer_rule<token> lexerRules[] =
    {
        { token::Document >> token::FunctionDefinition },
        { token::FunctionDefinition >> token::FunctionPrototype | token::FunctionBody },
        { token::FunctionPrototype >> 
            token::FunctionReturnType |
            token::FunctionName |  
            "("_ | token::FunctionParameter | repeat(sequence( ","_ | token::FunctionParameter )) | ")"_ },
        { token::Expression >> token::Expression | choice(token::Add | token::Subtract) | token::Term },
        { token::Expression >> token::Term },
        { token::Term >> token::Term | choice(token::Divide | token::Multiply) | token::Primary },
        { token::Term >> token::Primary },
        { token::Primary >> token::Number },
        { token::Primary >> token::Name },
        { token::Primary >> token::Name | token::Assign | token::Expression },
        { token::Primary >> token::Negate | token::Primary },
        { token::Primary >> "("_ | token::Expression | ")"_ },
        { token::Add >> "+"_ },
        { token::Subtract >> "-"_ },
        { token::Multiply >> "*"_ },
        { token::Divide >> "/"_ },
        { token::Negate >> "-"_ },
        { token::Assign >> ":="_ },
        { token::Equal >> "="_ },

        // whitespace handling...

        { token::Whitespace >> discard(optional(repeat(" "_ | "\r"_ | "\n"_ | "\t"_))) },
        { token::Document >> token::Whitespace | token::Document | token::Whitespace },
        { token::FunctionDefinition >> token::Whitespace | token::FunctionDefinition | token::Whitespace },
        { token::FunctionPrototype >> token::Whitespace | token::FunctionPrototype | token::Whitespace },
        { token::FunctionBody >> token::Whitespace | token::FunctionBody | token::Whitespace },
        { token::FunctionReturnType >> token::Whitespace | token::FunctionReturnType | token::Whitespace },
        { token::FunctionName >> token::Whitespace | token::FunctionName | token::Whitespace },
        { token::FunctionParameter >> token::Whitespace | token::FunctionParameter | token::Whitespace },
        { token::Expression >> token::Whitespace | token::Expression | token::Whitespace },
        { token::Term >> token::Whitespace | token::Term | token::Whitespace },
        { token::Primary >> token::Whitespace | token::Primary | token::Whitespace }
    };

    neolib::lexer<token> parser;
    auto result = parser.parse(source);
}

