#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <map>
#include <string>
#include <vector>

// A small hand-written JSON reader (no third-party library).
// Supports objects, arrays, strings, numbers, true/false/null.
// Any problem throws std::runtime_error with the position in the text.

class JsonValue
{
public:
    enum Type { Null, Bool, Number, String, Array, Object };

    Type type = Null;
    bool boolean = false;
    double number = 0.0;
    std::string text;
    std::vector<JsonValue> items;             // used when type == Array
    std::map<std::string, JsonValue> members; // used when type == Object

    bool isObject() const { return type == Object; }
    bool isArray() const { return type == Array; }

    // true if this is an object that has the key
    bool has(const std::string &key) const;

    // object lookup; throws if missing or this is not an object
    const JsonValue &at(const std::string &key) const;

    // typed getters; throw if the value has the wrong type
    int asInt() const;
    double asDouble() const;
    bool asBool() const;
    const std::string &asString() const;
};

// Parses a whole JSON document.
JsonValue parseJson(const std::string &input);

// Reads a file and parses it.
JsonValue parseJsonFile(const std::string &path);

#endif
