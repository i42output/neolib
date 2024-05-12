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
    FunctionParameter
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
        { token::Whitespace >> optional(repeat(" "_ | "\r"_ | "\n"_ | "\t"_)) },
        { token::Document >> token::Whitespace },
        { token::Document >> token::Whitespace | token::FunctionDefinition | token::Whitespace },
        { token::FunctionDefinition >> token::FunctionPrototype | token::Whitespace | token::FunctionBody },
        { token::FunctionPrototype >> 
            token::FunctionReturnType | token::Whitespace |
            token::FunctionName | token::Whitespace | 
            "("_ | token::Whitespace | token::FunctionParameter | 
            repeat(sequence(token::Whitespace | ","_ | token::Whitespace | token::FunctionParameter )) | 
            token::Whitespace | ")" }
    };

    neolib::lexer<token> parser;
    auto result = parser.parse(source);
}

