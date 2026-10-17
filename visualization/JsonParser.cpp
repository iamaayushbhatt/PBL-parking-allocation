#include "JsonParser.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

// ---------- JsonValue helpers ----------

bool JsonValue::has(const string &key) const
{
    return type == Object && members.count(key) > 0;
}

const JsonValue &JsonValue::at(const string &key) const
{
    if (type != Object)
        throw runtime_error("JSON: expected an object when looking up \"" + key + "\"");
    auto it = members.find(key);
    if (it == members.end())
        throw runtime_error("JSON: missing key \"" + key + "\"");
    return it->second;
}

int JsonValue::asInt() const
{
    if (type != Number)
        throw runtime_error("JSON: expected a number");
    if (number != floor(number))
        throw runtime_error("JSON: expected a whole number");
    return (int)number;
}

double JsonValue::asDouble() const
{
    if (type != Number)
        throw runtime_error("JSON: expected a number");
    return number;
}

bool JsonValue::asBool() const
{
    if (type != Bool)
        throw runtime_error("JSON: expected true or false");
    return boolean;
}

const string &JsonValue::asString() const
{
    if (type != String)
        throw runtime_error("JSON: expected a string");
    return text;
}

// ---------- the parser (recursive descent) ----------

namespace
{
class Parser
{
    const string &s;
    size_t pos = 0;

    [[noreturn]] void fail(const string &msg)
    {
        throw runtime_error("JSON error at position " + to_string(pos) + ": " + msg);
    }

    void skipSpaces()
    {
        while (pos < s.size() && isspace((unsigned char)s[pos]))
            pos++;
    }

    char peek()
    {
        skipSpaces();
        if (pos >= s.size())
            fail("unexpected end of input");
        return s[pos];
    }

    void expect(char c)
    {
        if (peek() != c)
            fail(string("expected '") + c + "'");
        pos++;
    }

    bool matchWord(const char *w)
    {
        size_t n = 0;
        while (w[n])
            n++;
        if (s.compare(pos, n, w) == 0)
        {
            pos += n;
            return true;
        }
        return false;
    }

    JsonValue parseValue()
    {
        char c = peek();
        if (c == '{')
            return parseObject();
        if (c == '[')
            return parseArray();
        if (c == '"')
        {
            JsonValue v;
            v.type = JsonValue::String;
            v.text = parseString();
            return v;
        }
        if (c == '-' || isdigit((unsigned char)c))
            return parseNumber();

        JsonValue v;
        if (matchWord("true"))
        {
            v.type = JsonValue::Bool;
            v.boolean = true;
            return v;
        }
        if (matchWord("false"))
        {
            v.type = JsonValue::Bool;
            v.boolean = false;
            return v;
        }
        if (matchWord("null"))
            return v;
        fail("unexpected character");
    }

    JsonValue parseObject()
    {
        JsonValue v;
        v.type = JsonValue::Object;
        expect('{');
        if (peek() == '}')
        {
            pos++;
            return v;
        }
        while (true)
        {
            if (peek() != '"')
                fail("expected a string key");
            string key = parseString();
            expect(':');
            v.members[key] = parseValue();
            char c = peek();
            pos++;
            if (c == '}')
                break;
            if (c != ',')
                fail("expected ',' or '}'");
        }
        return v;
    }

    JsonValue parseArray()
    {
        JsonValue v;
        v.type = JsonValue::Array;
        expect('[');
        if (peek() == ']')
        {
            pos++;
            return v;
        }
        while (true)
        {
            v.items.push_back(parseValue());
            char c = peek();
            pos++;
            if (c == ']')
                break;
            if (c != ',')
                fail("expected ',' or ']'");
        }
        return v;
    }

    string parseString()
    {
        expect('"');
        string out;
        while (true)
        {
            if (pos >= s.size())
                fail("string is not closed");
            char c = s[pos++];
            if (c == '"')
                break;
            if (c != '\\')
            {
                out += c;
                continue;
            }
            if (pos >= s.size())
                fail("bad escape");
            char e = s[pos++];
            switch (e)
            {
            case '"': out += '"'; break;
            case '\\': out += '\\'; break;
            case '/': out += '/'; break;
            case 'n': out += '\n'; break;
            case 't': out += '\t'; break;
            case 'r': out += '\r'; break;
            case 'b': out += '\b'; break;
            case 'f': out += '\f'; break;
            case 'u': // keep it simple: only \u00XX (ASCII) is supported
            {
                if (pos + 4 > s.size())
                    fail("bad \\u escape");
                int code = (int)strtol(s.substr(pos, 4).c_str(), nullptr, 16);
                pos += 4;
                if (code > 127)
                    fail("only ASCII \\u escapes are supported");
                out += (char)code;
                break;
            }
            default:
                fail("unknown escape");
            }
        }
        return out;
    }

    JsonValue parseNumber()
    {
        size_t start = pos;
        if (s[pos] == '-')
            pos++;
        while (pos < s.size() && (isdigit((unsigned char)s[pos]) || s[pos] == '.' ||
                                  s[pos] == 'e' || s[pos] == 'E' || s[pos] == '+' || s[pos] == '-'))
            pos++;
        JsonValue v;
        v.type = JsonValue::Number;
        char *end = nullptr;
        string piece = s.substr(start, pos - start);
        v.number = strtod(piece.c_str(), &end);
        if (end == piece.c_str() || *end != '\0')
            fail("bad number");
        return v;
    }

public:
    explicit Parser(const string &text) : s(text) {}

    JsonValue parseDocument()
    {
        JsonValue v = parseValue();
        skipSpaces();
        if (pos != s.size())
            fail("extra text after the JSON value");
        return v;
    }
};
} // namespace

JsonValue parseJson(const string &input)
{
    Parser p(input);
    return p.parseDocument();
}

JsonValue parseJsonFile(const string &path)
{
    ifstream in(path);
    if (!in)
        throw runtime_error("cannot open file: " + path);
    stringstream buffer;
    buffer << in.rdbuf();
    return parseJson(buffer.str());
}
