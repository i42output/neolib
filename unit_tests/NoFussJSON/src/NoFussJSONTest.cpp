#include <neolib/neolib.hpp>
#include <iostream>
#include <fstream>
#include <numeric>
#include <neolib/file/json.hpp>
#include <chrono>

#ifdef COMPARE_NOFUSSJSON_WITH_RAPIDJSON
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;
#endif


int main(int argc, char** argv)
{
    try
    {
        const std::string tests[] =
        {
            "\"foo\"",
            "\n\"foo\"\n",
            " \"foo\" ",
            " \"foo\" err",
            "\"foo\",\"err\"",
            "\"tab\\ttab\"",
            "\n\"tab\\ttab\"\n",
            " \"tab\\ttab\" ",
            " \"tab\\ttab\" err",
            "\"LF\\nLF\"",
            "\n\"LF\\nLF\"\n",
            " \"LF\\nLF\" ",
            " \"LF \\n LF\" ",
            " \"LF\\nLF\" err",
            "\"a\\tb\\nc\\td\"",
            "\n\"a\\tb\\nc\\td\"\n",
            " \"a\\tb\\nc\\td\" ",
            " \"a \\tb\\nc\\t d\" ",
            " \"a\\tb\\nc\\td\" err",
            "\"Q: \\u0051\"",
            "\"Omega: \\u03A9\"",
            "\"1 g clef 2 g clef 3: 1\\uD834\\uDD1E2\\uD834\\uDD1E3\"",
            "\"Error: \\u123\"",
            "\"Error: \\u123 \"",
            "\"Error: \\uZOOL\"",
            "0", "1", "4294967295", "281474976710656", "-1", "-281474976710656", "18446744073709551615", "0.1", "123456789012345678901234567890",
            "42",
            "\n42\n",
            " 42 ",
            " 42 err",
            "-42",
            "\n-42\n",
            " -42 ",
            " -42 err",
            "42e2",
            "\n42e2\n",
            " 42e2 ",
            " 42e2 err",
            "-42e2",
            "\n-42e2\n",
            " -42e2 ",
            " -42e2 err",
            "42e-2",
            "\n42e-2\n",
            " 42e-2 ",
            " 42e-2 err",
            "-42e-2",
            "\n-42e-2\n",
            " -42e-2 ",
            " -42e-2 err",
            "42.42",
            "\n42.42\n",
            " 42.42 ",
            " 42.42 err",
            "-42.42",
            "\n-42.42\n",
            " -42.42 ",
            " -42.42 err",
            "42.42e2",
            "\n42.42e2\n",
            " 42.42e2 ",
            " 42.42e2 err",
            "-42.42e2",
            "\n-42.42e2\n",
            " -42.42e2 ",
            " -42.42e2 err",
            "42.42e-2",
            "\n42.42e-2\n",
            " 42.42e-2 ",
            " 42.42e-2 err",
            "-42.42e-2",
            "\n-42.42e-2\n",
            " -42.42e-2 ",
            " -42.42e-2 err",
            "true",
            "\ntrue\n",
            " true ",
            " true err",
            "false",
            "\nfalse\n",
            " false ",
            " false err",
            "null",
            "\nnull\n",
            " null ",
            " null err",
            "[]",
            "[[],[],[]]",
            "[1 ]",
            "[ 1]",
            "[ 1 ]",
            "[1,2,3]",
            "[1,2,3,\"foo\", 42 , \"bar\", true, false, null]",
            "[1,2,3,[\"a\",\"b\",\"c\"],4,5,6]",
            "[1,]",
            "[1,,]",
            "[,2,]",
            "[,]",
            "[,,]",
            "{}",
            "{ \"test\": 42 }",
            "{ \"test\": 42, \"foo\": \"bar\" }",
            "{ \"test\": 42, \"obj\": { \"foo\": \"bar\" } }"
        };
        for (const auto& test : tests)
        {
            std::cout << "----Test-------------------" << std::endl;
            std::cout << test;
            try
            {
                std::istringstream testStream{ test };
                std::cout << "\n----Parsing----------------" << std::endl;
                neolib::fast_json json{ testStream };
                std::cout << "\n----Result-----------------" << std::endl;
                std::cout << "Root type: " << neolib::to_string(json.root().type()) << std::endl;
                json.write(std::cout);
                std::cout << std::endl;
            }
            catch (std::exception& e)
            {
                std::cout << "\n****Parse Error***********" << std::endl;
                std::cerr << e.what() << std::endl;
            }
            std::cout << "---------------------------" << std::endl;
        }

        const std::string JSON_at_test =
        {
            "{\n"
            "   \"foo\" : {\n"
            "       \"bar\" : {\n"
            "           \"baz\" : {\n"
            "               \"test\": \"wibble\"\n"
            "           }\n"
            "       }\n"
            "   }\n"
            "}\n"
        };
        std::cout << "----JSON at-input---------------------" << std::endl;
        std::cout << JSON_at_test;
        std::cout << "----JSON at-result---------------------" << std::endl;
        std::istringstream jsonAtStream(JSON_at_test);
        neolib::json jsonAt{ jsonAtStream };
        std::cout << ".at(\"foo.bar.baz.test)\" == " << jsonAt.at("foo.bar.baz.test").text() << std::endl;
        std::cout << std::endl;
        std::cout << "----JSON at ends-----------------------" << std::endl;

        const std::string RJSON_test =
        {
            "{\n"
            "  // This is a sample RJSON file\n"
            "\n"
            "  buy: [milk eggs butter 'dog bones']\n"
            "  quotey: \"foo\"/*bar*/\n"
            "  quotey: \"foo\" /*bar*/\n"
            "  quotey: \"foo\"//bar\n"
            "  quotey: \"foo\" //bar\n"
            "  tasks : [{name:exercise completed : false} {name:eat completed : true}]\n"
            "\n"
            "  'another key' : 'another value'\n"
            "\n"
            "/*  It is very easy\n"
            "to read and write RJSON\n"
            "without quotes or commas!\n"
            "*/\n"
            "}\n"
        };
        std::cout << "----RJSON-input---------------------" << std::endl;
        std::cout << RJSON_test;
        std::cout << "----RJSON-output---------------------" << std::endl;
        std::istringstream rjsonStream(RJSON_test);
        neolib::rjson rjson{ rjsonStream };
        rjson.write(std::cout);
        std::cout << std::endl; 
        std::cout << "----RJSON ends-----------------------" << std::endl;

        const std::string FJSON_test =
        {
            "{\n"
            "  default_size: [ 800spx 800spx ]\n"
            "}\n"
        };
        std::cout << "----FJSON-input---------------------" << std::endl;
        std::cout << FJSON_test;
        std::cout << "----FJSON-output---------------------" << std::endl;
        std::istringstream fjsonStream(FJSON_test);
        neolib::fjson fjson{ fjsonStream };
        fjson.write(std::cout);
        std::cout << std::endl;
        std::cout << "----FJSON ends-----------------------" << std::endl;

        std::cout << "------ code ------" << std::endl;
        neolib::json json;
        json.root() = neolib::json_object{};
        json.root().as<neolib::json_object>()["answer"] = 42;
        for (auto& c : json.root())
            ;
        double arithmeticConversionCheck = json.croot().as<neolib::json_object>().at("answer").as<double>();
        json.write(std::cout);
        std::cout << "\n------------------" << std::endl;
        std::cout << "arithmeticConversionCheck: " << arithmeticConversionCheck << std::endl;

        std::string input;
        if (argc < 2)
        {
            std::cout << "Input: ";
            std::cin >> input;
        }
        else
            input = argv[1];

        try
        {
            neolib::fast_json json{ input };
            std::cout << "Write:" << std::endl;
            json.write(std::cout);
            std::cout << "\nVisit:" << std::endl;
            json.visit([](auto&& arg)
            {
                if constexpr(std::is_same_v<typename std::remove_cv<typename std::remove_reference<decltype(arg)>::type>::type, neolib::none_t>)
                    return;
                else if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, neolib::fast_json_object>)
                    std::cout << "(object)" << std::endl;
                else if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, neolib::fast_json_array>)
                    std::cout << "(array)" << std::endl;
                else if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, neolib::fast_json_null>)
                    std::cout << "null" << std::endl;
                else if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, neolib::fast_json_keyword>)
                    std::cout << "(keyword)" << std::endl;
                else if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, std::monostate>)
                    std::cout << "(empty" << std::endl;
                else
                    std::cout << arg << std::endl;
            });
            std::string output;
            if (argc < 3)
            {
                std::cout << "Output: ";
                std::cin >> output;
            }
            else
                output = argv[2];

            json.write(output);

        }
        catch (std::exception& e)
        {
            std::cout << "\n****Parse Error***********" << std::endl;
            std::cerr << e.what() << std::endl;
        }

        std::string inputBenchmark;
        if (argc < 4)
        {
            std::cout << "Input (benchmark): ";
            std::cin >> inputBenchmark;
        }
        else
            inputBenchmark = argv[3];

        {
            std::vector<uint64_t> timings;
            for (int i = 0; i < 100; ++i)
            {
                auto start_time = std::chrono::high_resolution_clock::now();
                {
                    neolib::json json{ inputBenchmark };
                }
                auto end_time = std::chrono::high_resolution_clock::now();
                timings.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
            }
            auto average = std::accumulate(timings.begin(), timings.end(), 0ull) / timings.size();
            std::cout << "Average (NoFussJSON default): " << average << std::endl;
        }
        {
            std::vector<uint64_t> timings;
            for (int i = 0; i < 100; ++i)
            {
                auto start_time = std::chrono::high_resolution_clock::now();
                {
                    typedef neolib::basic_json<neolib::json_syntax::Standard, neolib::omega_pool_allocator<neolib::json_type, 3 * 20 * 1024 * 1024>> omega_json;
                    if (i > 0)
                        omega_json::json_value::value_allocator().omega_recycle();
                    omega_json json{ inputBenchmark };
                    if (i == 0)
                        omega_json::json_value::value_allocator().info(std::cout);
                }
                auto end_time = std::chrono::high_resolution_clock::now();
                timings.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
            }
            auto average = std::accumulate(timings.begin(), timings.end(), 0ull) / timings.size();
            std::cout << "Average (NoFussJSON omega): " << average << std::endl;
        }
        {
            std::vector<uint64_t> timings;
            for (int i = 0; i < 100; ++i)
            {
                auto start_time = std::chrono::high_resolution_clock::now();
                {
                    neolib::fast_json json{ inputBenchmark };
                }
                auto end_time = std::chrono::high_resolution_clock::now();
                timings.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
            }
            auto average = std::accumulate(timings.begin(), timings.end(), 0ull) / timings.size();
            std::cout << "Average (NoFussJSON fast): " << average << std::endl;
        }
#ifdef COMPARE_NOFUSSJSON_WITH_RAPIDJSON
        {
            std::vector<uint64_t> timings;
            for (int i = 0; i < 100; ++i)
            {
                auto start_time = std::chrono::high_resolution_clock::now();
                {
                    FILE* fp = fopen(inputBenchmark.c_str(), "r");
                    fseek(fp, 0, SEEK_END);
                    size_t filesize = (size_t)ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    char* buffer = (char*)malloc(filesize + 1);
                    size_t readLength = fread(buffer, 1, filesize, fp);
                    buffer[readLength] = '\0';
                    fclose(fp);
                    // In situ parsing the buffer into d, buffer will also be modified
                    Document d;
                    d.ParseInsitu(buffer);
                    // Query/manipulate the DOM here...
                    free(buffer);
                }
                auto end_time = std::chrono::high_resolution_clock::now();
                timings.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
            }
            auto average = std::accumulate(timings.begin(), timings.end(), 0ull) / timings.size();
            std::cout << "Average (RapidJSON): " << average << std::endl;
        }
#endif
    }
    catch (std::exception& e)
    {
        std::cerr << "\nError: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

