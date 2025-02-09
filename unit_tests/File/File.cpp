#include <neolib/neolib.hpp>
#include <neolib/file/parser.hpp>

namespace
{
    void test_assert(bool assertion)
    {
        if (!assertion)
            throw std::logic_error("Test failed");
    }
    void test_assert(int iteration, bool assertion)
    {
        if (!assertion)
        {
            std::cerr << "Test failed, iteration = " << iteration << std::endl;
            throw std::logic_error("Test failed");
        }
    }
}

namespace parser_test
{
    enum class symbol
    {
        Program,
        Whitespace,
        Comment,
        Identifier,
        FunctionDefinition,
        FunctionPrototype,
        FunctionBody,
        FunctionReturnType,
        FunctionName,
        FunctionParameterList,
        FunctionParameterListOpen,
        FunctionParameterListClose,
        FunctionParameter,
        OpenScope,
        CloseScope,
        Type,
        Statement,
        EndStatement,
        Expression,
        OpenExpression,
        CloseExpression,
        Term,
        Factor,
        Primary,
        Add,
        Subtract,
        Multiply,
        Divide,
        Negate,
        Integer,
        Float,
        Number,
        Minus,
        Digit,
        Decimal,
        Variable,
        Assign,
        Equal,
    };
}

declare_symbols(parser_test::symbol)
declare_symbol(parser_test::symbol, Program)
declare_symbol(parser_test::symbol, Whitespace)
declare_symbol(parser_test::symbol, Comment)
declare_symbol(parser_test::symbol, Identifier)
declare_symbol(parser_test::symbol, FunctionDefinition)
declare_symbol(parser_test::symbol, FunctionPrototype)
declare_symbol(parser_test::symbol, FunctionBody)
declare_symbol(parser_test::symbol, FunctionReturnType)
declare_symbol(parser_test::symbol, FunctionName)
declare_symbol(parser_test::symbol, FunctionParameterList)
declare_symbol(parser_test::symbol, FunctionParameterListOpen)
declare_symbol(parser_test::symbol, FunctionParameterListClose)
declare_symbol(parser_test::symbol, FunctionParameter)
declare_symbol(parser_test::symbol, OpenScope)
declare_symbol(parser_test::symbol, CloseScope)
declare_symbol(parser_test::symbol, Type)
declare_symbol(parser_test::symbol, Statement)
declare_symbol(parser_test::symbol, EndStatement)
declare_symbol(parser_test::symbol, Expression)
declare_symbol(parser_test::symbol, OpenExpression)
declare_symbol(parser_test::symbol, CloseExpression)
declare_symbol(parser_test::symbol, Term)
declare_symbol(parser_test::symbol, Factor)
declare_symbol(parser_test::symbol, Primary)
declare_symbol(parser_test::symbol, Add)
declare_symbol(parser_test::symbol, Subtract)
declare_symbol(parser_test::symbol, Multiply)
declare_symbol(parser_test::symbol, Divide)
declare_symbol(parser_test::symbol, Negate)
declare_symbol(parser_test::symbol, Integer)
declare_symbol(parser_test::symbol, Float)
declare_symbol(parser_test::symbol, Number)
declare_symbol(parser_test::symbol, Minus)
declare_symbol(parser_test::symbol, Digit)
declare_symbol(parser_test::symbol, Decimal)
declare_symbol(parser_test::symbol, Variable)
declare_symbol(parser_test::symbol, Assign)
declare_symbol(parser_test::symbol, Equal)
end_declare_symbols(parser_test::symbol);

namespace parser_test
{
    enable_neolib_parser(symbol)
}

std::string_view const sourcePass1 = R"test(r f(){42!;})test";

std::string_view const sourcePass2 = R"test(
    xyzzY0 foo()
    {
        1234; /* comment one */
        x := 1 + 2 + 3 - 4 - 5 + 6; // comment two
        y := 7 + -42.001 * 1.0 * (5-1+2) + -x + x * 2;
    }
)test";

std::string_view const sourceError1 = R"test(r f(){42.0!;})test";

std::string_view const sourceError2 = R"test(
    xyzzY0 foo()
    {
        1234q;
        x := 1 + 2 + 3 - 4 - 5 + 6; 
        y := 7 + -42.001 * 1.0 * (5-1+2) + -x + x * 2;
    }
)test";

std::string_view const sourceError3 = R"test(
    xyzzY0 foo()
    {
        1234;
        x := 1 + 2 + 3 - 4 - 5 + 6; 
        y := 7 + 4
2.0 * 1.0 * (5-1+2) + -x + x * 2;
    }
)test";

std::string_view const sourceError4 = R"test(
    xyzzY0 foo()
    {
        1234;
        x := 1 + 2 + 3 - 4 - 5 + 6; 
        y := 7 + -42.001 * 1.0 * (5-1+2)) + -x + x * 2;
    }
)test";

int main(int argc, char** argv)
{
    using namespace parser_test;

    auto const WS = symbol::Whitespace;

    neolib::parser_rule<symbol> parserRules[] =
    {
        ( symbol::Program >> repeat(symbol::FunctionDefinition) ),
        ( symbol::FunctionDefinition >> WS , symbol::FunctionPrototype , WS, symbol::FunctionBody , WS ),
        ( symbol::FunctionPrototype >> WS, symbol::FunctionReturnType , WS, symbol::FunctionName , WS , symbol::FunctionParameterList , WS ),
        ( symbol::FunctionReturnType >> symbol::Type ),
        ( symbol::FunctionName >> symbol::Identifier ),
        ( symbol::FunctionParameterList >>
            WS ,
            ~discard(symbol::FunctionParameterListOpen) , 
            WS , 
            optional(sequence(WS , symbol::FunctionParameter, WS , repeat(sequence(WS , ',' , WS , symbol::FunctionParameter , WS)))) , 
            WS ,
            ~discard(symbol::FunctionParameterListClose) ),
        ( symbol::FunctionParameterListOpen >> '(' ),
        ( symbol::FunctionParameterListClose >> ')' ),
        ( symbol::FunctionParameter >> symbol::Type, WS , symbol::Variable ),
        ( symbol::FunctionBody >> (WS , ~discard(symbol::OpenScope) , WS , repeat(symbol::Statement) , WS , ~discard(symbol::CloseScope) , WS) ),
        ( symbol::Type >> symbol::Identifier ),
        ( symbol::Identifier >> (+repeat(range('A', 'Z') | range('a', 'z')) , 
            repeat(range('A', 'Z') | range('a', 'z') | range('0', '9'))) ),
        ( symbol::OpenScope >> '{' ),
        ( symbol::CloseScope >> '}' ),
        ( symbol::Statement >> WS , symbol::Expression , WS , discard(symbol::EndStatement) , WS ),
        ( symbol::EndStatement >> ';' ),
        ( symbol::Expression >> (WS , symbol::Term , WS , 
            +repeat(sequence(
                WS ,
                symbol::Add <=> "math.operator.add"_infix_concept | 
                symbol::Subtract <=> "math.operator.subtract"_infix_concept , WS , symbol::Term , WS) <=> "math.addition"_concept)) ),
        ( symbol::Expression >> symbol::Term ),
        ( symbol::Term >> (WS , symbol::Factor , WS , 
            +repeat(sequence(
                WS ,
                symbol::Multiply <=> "math.operator.multiply"_infix_concept | 
                symbol::Divide <=> "math.operator.divide"_infix_concept , WS , symbol::Factor , WS) <=> "math.multiplication"_concept)) ),
        ( symbol::Term >> symbol::Factor ),
        ( symbol::Factor >> symbol::Primary ),
        ( symbol::Primary >> 
            ((symbol::Variable <=> "object"_concept , WS , symbol::Assign , WS , symbol::Expression) <=> "object.assign"_concept)),
        ( symbol::Primary >> symbol::Number ),
        ( symbol::Primary >> ((symbol::Negate , symbol::Primary) <=> "math.operator.negate"_concept) ),
        ( symbol::Primary >> (sequence(symbol::Integer , WS , '!') <=> "math.operator.factorial"_concept) ),
        ( symbol::Primary >> (symbol::Variable <=> "object"_concept) ),
        ( symbol::Primary >> ~discard(symbol::OpenExpression) , WS , symbol::Expression , WS , ~discard(symbol::CloseExpression) ),
        ( symbol::OpenExpression >> '(' ),
        ( symbol::CloseExpression >> ')' ),
        ( symbol::Add >> '+' ),
        ( symbol::Subtract >> '-' ),
        ( symbol::Multiply >> '*' ),
        ( symbol::Divide >> '/' ),
        ( symbol::Negate >> '-' ),
        ( symbol::Assign >> ":=" ),
        ( symbol::Equal >> '=' ),
        ( symbol::Minus >> '-' ),
        ( symbol::Number >> (symbol::Float | symbol::Integer) ),
        ( symbol::Float >> (fold((optional(symbol::Minus), +repeat(symbol::Digit) , symbol::Decimal, +repeat(symbol::Digit))) <=> "number.float"_concept) ),
        ( symbol::Integer >> (fold((optional(symbol::Minus), +repeat(symbol::Digit))) <=> "number.integer"_concept) ),
        ( symbol::Digit >> range('0' , '9') ),
        ( symbol::Decimal >> '.' ),
        ( symbol::Variable >> symbol::Identifier ),

        // whitespace handling...

        ( symbol::Whitespace >> repeat(' '_ | '\r' | '\n' | '\t' | symbol::Comment) ),
        ( symbol::Comment >> sequence("/*"_ , repeat(
            repeat(range('\x00', '\x29') | range('\x2B', '\xFF')) | 
            ("*"_ , repeat(range('\x00', '\x2E') | range('\x30', '\xFF')))) , "*/"_)),
        ( symbol::Comment >> sequence("//"_ , repeat(range('\x00', '\x09') | range('\x0B', '\xFF')) , "\n"_) )
    };

    neolib::parser<symbol> parser{ parserRules };
    parser.ignore(symbol::Whitespace);
    parser.set_debug_output(std::cerr, false, true);
    parser.set_debug_scan(false);
    test_assert(parser.parse(symbol::Program, sourcePass1));
    parser.create_ast();
    test_assert(parser.parse(symbol::Program, sourcePass2));
    parser.create_ast();
    test_assert(!parser.parse(symbol::Program, sourceError1));
    test_assert(!parser.parse(symbol::Program, sourceError2));
    test_assert(!parser.parse(symbol::Program, sourceError3));
    test_assert(!parser.parse(symbol::Program, sourceError4));
}

