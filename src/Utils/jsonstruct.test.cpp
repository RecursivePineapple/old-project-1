
#pragma target server-test

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Utils/jsonparser.hpp"
#include "Utils/jsonstruct.hpp"

using namespace std::literals::string_literals;

class JsonParserTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(JsonParserTest);
    CPPUNIT_TEST(Test_Parsing);
    CPPUNIT_TEST(Test_Parsing2);
    CPPUNIT_TEST(Test_ParsingLarge);
    CPPUNIT_TEST(Test_ParseValue);
    CPPUNIT_TEST(Test_ParseString);
    CPPUNIT_TEST(Test_ParseBool);
    CPPUNIT_TEST(Test_ParseInt);
    CPPUNIT_TEST(Test_ParseFloat);
    CPPUNIT_TEST(Test_ParseFloat2);
    CPPUNIT_TEST(Test_ParseDecimal);
    CPPUNIT_TEST(Test_ParseNullable);
    CPPUNIT_TEST(Test_ParseNullable2);
    CPPUNIT_TEST(Test_ParseArray);
    CPPUNIT_TEST(Test_ParseObject);
    CPPUNIT_TEST(Test_ParsePairs);
    CPPUNIT_TEST(Test_ParseSpan);
    CPPUNIT_TEST(Test_ParseSpan2);
    CPPUNIT_TEST(Test_ParseVariant);
    CPPUNIT_TEST(Test_ParseVariant2);
    CPPUNIT_TEST(Test_ParseValue);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {}
    void tearDown(void) {}

    void Test_Parsing(void) {
        std::string str = "{\"hello\":5}";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 3 tokens", 3ul, toks.size());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be obj", JSMN_OBJECT, toks[0].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 1 should be str", JSMN_STRING, toks[1].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 2 should be int", JSMN_PRIMITIVE, toks[2].type);
    }

    void Test_Parsing2(void) {
        std::string str = "{\"hello\":{\"world\":5}}";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 5 tokens", 5ul, toks.size());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be obj", JSMN_OBJECT, toks[0].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 1 should be str", JSMN_STRING, toks[1].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 3 should be obj", JSMN_OBJECT, toks[2].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 4 should be str", JSMN_STRING, toks[3].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 5 should be int", JSMN_PRIMITIVE, toks[4].type);
    }

    void Test_ParsingLarge(void) {
        std::string str = "[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 51 tokens", 51ul, toks.size());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be array", JSMN_ARRAY, toks[0].type);

        for(size_t i = 1; i < toks.size(); i++)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("rest toks should be int", JSMN_PRIMITIVE, toks[i].type);
        }
    }

    void Test_ParseString(void) {
        std::string str = "\"hello\"";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 token", 1ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be string", JSMN_STRING, toks[0].type);

        std::string value;
        bool success = jsontypes::string::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be correct", "hello"s, value);

        std::stringstream ss;
        jsontypes::string::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParseBool(void) {
        std::string str = "true";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 token", 1ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be primitive", JSMN_PRIMITIVE, toks[0].type);

        bool value;
        bool success = jsontypes::boolean::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be correct", true, value);

        std::stringstream ss;
        jsontypes::boolean::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParseInt(void) {
        std::string str = "123";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 token", 1ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be primitive", JSMN_PRIMITIVE, toks[0].type);

        int value;
        bool success = jsontypes::integer::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be correct", 123, value);

        std::stringstream ss;
        jsontypes::integer::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParseFloat(void) {
        std::string str = "1.23";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 token", 1ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be primitive", JSMN_PRIMITIVE, toks[0].type);

        double value;
        bool success = jsontypes::floating::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be correct", 1.23, value);

        std::stringstream ss;
        jsontypes::floating::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParseFloat2(void) {
        std::string str = "12e3";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 token", 1ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be primitive", JSMN_PRIMITIVE, toks[0].type);

        double value;
        bool success = jsontypes::floating::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be correct", 12000.0, value);

        std::stringstream ss;
        jsontypes::floating::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", "12000"s, ss.str());
    }

    void Test_ParseDecimal(void) {
        std::string str = "123.123123123123";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 token", 1ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be primitive", JSMN_PRIMITIVE, toks[0].type);

        decimal_t value;
        bool success = jsontypes::decimal::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be correct", "123.123123123123"s, value.to_string());

        std::stringstream ss;
        jsontypes::decimal::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParseNullable(void) {
        std::string str = "null";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 token", 1ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be primitive", JSMN_PRIMITIVE, toks[0].type);

        jsontypes::nullable<jsontypes::integer>::ctype value;
        bool success = jsontypes::nullable<jsontypes::integer>::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_MESSAGE("should be null", value == nullptr);

        std::stringstream ss;
        jsontypes::nullable<jsontypes::integer>::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParseNullable2(void) {
        std::string str = "5";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 token", 1ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be primitive", JSMN_PRIMITIVE, toks[0].type);

        jsontypes::nullable<jsontypes::integer>::ctype value;
        bool success = jsontypes::nullable<jsontypes::integer>::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_MESSAGE("should not be null", value == 5);

        std::stringstream ss;
        jsontypes::nullable<jsontypes::integer>::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParseArray(void) {
        std::string str = "[1,2]";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 3 tokens", 3ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be array", JSMN_ARRAY, toks[0].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 1 should be primitive", JSMN_PRIMITIVE, toks[1].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 2 should be primitive", JSMN_PRIMITIVE, toks[2].type);

        jsontypes::array<jsontypes::integer>::ctype value;
        bool success = jsontypes::array<jsontypes::integer>::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 2 items", 2ul, value.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("[0] should be 1", 1, value[0]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("[1] should be 2", 2, value[1]);

        std::stringstream ss;
        jsontypes::array<jsontypes::integer>::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParseObject(void) {
        std::string str = "{\"hello\":5,\"world\":6}";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 5 tokens", 5ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be object", JSMN_OBJECT, toks[0].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 1 should be string", JSMN_STRING, toks[1].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 2 should be primitive", JSMN_PRIMITIVE, toks[2].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 3 should be string", JSMN_STRING, toks[3].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 4 should be primitive", JSMN_PRIMITIVE, toks[4].type);

        jsontypes::object<jsontypes::string, jsontypes::integer>::ctype value;
        bool success = jsontypes::object<jsontypes::string, jsontypes::integer>::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 2 items", 2ul, value.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("[\"hello\"] should be 5", 5, value["hello"]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("[\"world\"] should be 6", 6, value["world"]);

        std::stringstream ss;
        jsontypes::object<jsontypes::string, jsontypes::integer>::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParsePairs(void) {
        std::string str = "{\"hello\":5,\"world\":6}";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 5 tokens", 5ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be object", JSMN_OBJECT, toks[0].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 1 should be string", JSMN_STRING, toks[1].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 2 should be primitive", JSMN_PRIMITIVE, toks[2].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 3 should be string", JSMN_STRING, toks[3].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 4 should be primitive", JSMN_PRIMITIVE, toks[4].type);

        jsontypes::pairs<jsontypes::string, jsontypes::integer>::ctype value;
        bool success = jsontypes::pairs<jsontypes::string, jsontypes::integer>::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 2 items", 2ul, value.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("0.0 should be \"hello\"", "hello"s, value[0].first);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("0.1 should be 5", 5, value[0].second);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("1.0 should be \"world\"", "world"s, value[1].first);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("1.1 should be 6", 6, value[1].second);

        std::stringstream ss;
        jsontypes::pairs<jsontypes::string, jsontypes::integer>::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParseSpan(void) {
        std::string str = "{\"hello\":{\"world\":5}}";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 5 tokens", 5ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be object", JSMN_OBJECT, toks[0].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 1 should be string", JSMN_STRING, toks[1].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 2 should be object", JSMN_OBJECT, toks[2].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 3 should be string", JSMN_STRING, toks[3].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 4 should be primitive", JSMN_PRIMITIVE, toks[4].type);

        jsontypes::object<jsontypes::string, jsontypes::span>::ctype value;
        bool success = jsontypes::object<jsontypes::string, jsontypes::span>::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 item", 1ul, value.size());

        auto span = value["hello"];
        
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 3 tokens", 3ul, span.toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be object", JSMN_OBJECT, span.toks[0].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 1 should be string", JSMN_STRING, span.toks[1].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 2 should be primitive", JSMN_PRIMITIVE, span.toks[2].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("text should be correct", "{\"world\":5}"s, span.str);

        jsontypes::object<jsontypes::string, jsontypes::integer>::ctype value2;
        bool success2 = span.ParseInto<jsontypes::object<jsontypes::string, jsontypes::integer>>(value2);

        CPPUNIT_ASSERT_MESSAGE("should be parseinto-able", success2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 item", 1ul, value2.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("[\"world\"] should be 5", 5, value2["world"]);
    }

    void Test_ParseSpan2(void) {
        jsontypes::span_t span;

        {
            std::string str = "{\"hello\":{\"world\":5}}";

            std::vector<jsmntok_t> toks;
            
            CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 5 tokens", 5ul, toks.size());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be object", JSMN_OBJECT, toks[0].type);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 1 should be string", JSMN_STRING, toks[1].type);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 2 should be object", JSMN_OBJECT, toks[2].type);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 3 should be string", JSMN_STRING, toks[3].type);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 4 should be primitive", JSMN_PRIMITIVE, toks[4].type);

            jsontypes::object<jsontypes::string, jsontypes::span>::ctype value;
            bool success = jsontypes::object<jsontypes::string, jsontypes::span>::parse(toks.data(), 0, str.data(), value);

            CPPUNIT_ASSERT_MESSAGE("should be parsable", success);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 item", 1ul, value.size());

            span = value["hello"];
        }
        
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 3 tokens", 3ul, span.toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be object", JSMN_OBJECT, span.toks[0].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 1 should be string", JSMN_STRING, span.toks[1].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 2 should be primitive", JSMN_PRIMITIVE, span.toks[2].type);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("text should be correct", "{\"world\":5}"s, span.str);

        jsontypes::object<jsontypes::string, jsontypes::integer>::ctype value2;
        bool success2 = span.ParseInto<jsontypes::object<jsontypes::string, jsontypes::integer>>(value2);

        CPPUNIT_ASSERT_MESSAGE("should be parseinto-able", success2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 item", 1ul, value2.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("[\"world\"] should be 5", 5, value2["world"]);
    }

    void Test_ParseVariant(void) {
        std::string str = "5";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 tokens", 1ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be object", JSMN_PRIMITIVE, toks[0].type);

        jsontypes::variant<jsontypes::integer, jsontypes::string>::ctype value;
        bool success = jsontypes::variant<jsontypes::integer, jsontypes::string>::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);

        std::optional<int> value2;

        std::visit([&value2](auto const& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr(std::is_same_v<T, int>) {
                value2 = arg;
            }
            if constexpr(std::is_same_v<T, std::string>) {
                CPPUNIT_FAIL("should not be a string");
            }
        }, value);

        CPPUNIT_ASSERT_MESSAGE("should be an int", value2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be 5", 5, value2.value());

        std::stringstream ss;
        jsontypes::variant<jsontypes::integer, jsontypes::string>::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParseVariant2(void) {
        std::string str = "\"hello\"";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 1 tokens", 1ul, toks.size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("tok 0 should be object", JSMN_STRING, toks[0].type);

        jsontypes::variant<jsontypes::integer, jsontypes::string>::ctype value;
        bool success = jsontypes::variant<jsontypes::integer, jsontypes::string>::parse(toks.data(), 0, str.data(), value);

        CPPUNIT_ASSERT_MESSAGE("should be parsable", success);

        std::optional<std::string> value2;

        std::visit([&value2](auto const& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr(std::is_same_v<T, std::string>) {
                value2 = arg;
            }
            if constexpr(std::is_same_v<T, int>) {
                CPPUNIT_FAIL("should not be an int");
            }
        }, value);

        CPPUNIT_ASSERT_MESSAGE("should be a string", value2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be \"hello\"", "hello"s, value2.value());

        std::stringstream ss;
        jsontypes::variant<jsontypes::integer, jsontypes::string>::emit(ss, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("emission should be correct", str, ss.str());
    }

    void Test_ParseValue(void) {
        std::string str = "{\"hello\":5}";

        std::vector<jsmntok_t> toks;
        
        CPPUNIT_ASSERT_MESSAGE("json should be valid", jsontypes::ParseJson(toks, str, true));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("should have 3 tokens", 3ul, toks.size());

        int value;
        bool success = server::ParseValue<jsontypes::integer, int>(toks, 0, str, "hello", value);
        CPPUNIT_ASSERT_MESSAGE("should find int hello", success);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("value should be 5", 5, value);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(JsonParserTest);
