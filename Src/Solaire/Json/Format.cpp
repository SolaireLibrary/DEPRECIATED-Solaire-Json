//Copyright 2015 Adam Smith
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

// Contact :
// Email             : solairelibrary@mail.com
// GitHub repository : https://github.com/SolaireLibrary/SolaireCPP

#include "Solaire/Json/Format.hpp"

namespace Solaire {

    static bool skipWhitespace(IStream& aStream) {
        if(aStream.end()) return true;
        char c;
        aStream >> c;
        while(std::isspace(c)) {
            if(aStream.end()) return true;
            aStream >> c;
        }
        aStream.setOffset(aStream.getOffset() - 1);
        return true;
    }

    static bool writeValue(const GenericValue& aValue, OStream& aStream) throw();

    static bool writeNull(OStream& aStream) throw() {
        aStream << "null";
        return true;
    }

    static bool writeChar(const char aValue, OStream& aStream) throw() {
        aStream << '"' << aValue << '"';
        return true;
    }

    static bool writeBool(const bool aValue, OStream& aStream) throw() {
        if(aValue) {
            aStream << "true";
        }else {
            aStream << "false";
        }
        return true;
    }

    static bool writeUnsigned(const uint64_t aValue, OStream& aStream) throw() {
        CString tmp;
        tmp += aValue;
        aStream << tmp;
        return true;
    }

    static bool writeSigned(const int64_t aValue, OStream& aStream) throw() {
        CString tmp;
        tmp += aValue;
        aStream << tmp;
        return true;
    }

    static bool writeDouble(const double aValue, OStream& aStream) throw() {
        //! \todo Implement writeDouble
        return false;
    }

    static bool writeString(const StringConstant<char>& aValue, OStream& aStream) throw() {
        aStream << '"';
        aStream << aValue;
        aStream << '"';
        return true;
    }

    static bool writeArray(const GenericArray& aValue, OStream& aStream) throw() {
        aStream << '[';
        const auto end = aValue.end();
        for(auto i = aValue.begin(); i != end; ++i) {
            if(! writeValue(*i, aStream)) return false;
            if(end - i > 1) aStream << ',';
        }
        aStream << ']';
        return true;
    }

    static bool writeObject(const GenericObject& aValue, OStream& aStream) throw() {
        aStream << '{';
        const SharedAllocation<StaticContainer<GenericObject::Entry>> entries = aValue.getEntries();
        const auto end = entries->end();
        for(auto i = entries->begin(); i != end; ++i) {
            if(! writeString(i->first, aStream)) return false;
            aStream << ':';
            if(! writeValue(i->second, aStream)) return false;
            if(end - i > 1) aStream << ',';
        }
        aStream << '}';
        return false;
    }

    static bool writeValue(const GenericValue& aValue, OStream& aStream) throw() {
        switch(aValue.getType()) {
        case GenericValue::NULL_T:
            return writeNull(aStream);
        case GenericValue::CHAR_T:
            return writeChar(aValue.getChar(), aStream);
        case GenericValue::BOOL_T:
            return writeBool(aValue.getBool(), aStream);
        case GenericValue::UNSIGNED_T:
            return writeUnsigned(aValue.getUnsigned(), aStream);
        case GenericValue::SIGNED_T:
            return writeSigned(aValue.getSigned(), aStream);
        case GenericValue::DOUBLE_T:
            return writeDouble(aValue.getDouble(), aStream);
        case GenericValue::STRING_T:
            return writeString(aValue.getString(), aStream);
        case GenericValue::ARRAY_T:
            return writeArray(aValue.getArray(), aStream);
        case GenericValue::OBJECT_T:
            return writeObject(aValue.getObject(), aStream);
        default:
            return false;
        }
    }

    static GenericValue::ValueType getType(IStream& aStream) {
        const int32_t offset = aStream.getOffset();
        GenericValue::ValueType type = GenericValue::NULL_T;

        skipWhitespace(aStream);
        if(! aStream.end()) {
            char c;
            aStream >> c;

            switch(c){
            case 'n':
                type = GenericValue::NULL_T;
                break;
            case 't':
            case 'f':
                type = GenericValue::BOOL_T;
                break;
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                type = GenericValue::DOUBLE_T;
                break;
            case '"':
                type = GenericValue::STRING_T;
                break;
            case '[':
                type = GenericValue::ARRAY_T;
                break;
            case '{':
                type = GenericValue::OBJECT_T;
                break;
            default:
                break;
            }
        }

        aStream.setOffset(offset);
        return type;
    }

    static GenericValue readValue(IStream& aStream);

    static GenericValue readNull(IStream& aStream) throw() {
        if(! skipWhitespace(aStream)) throw std::runtime_error("Json::Null : Could not locate value");
        char buffer[4];
        aStream.read(buffer, 4);
        if(std::memcmp(buffer, "null", 4) != 0) throw std::runtime_error("Json::Null : Value must be either 'null'");
        return GenericValue();
    }

    static GenericValue readBool(IStream& aStream) {
        if(! skipWhitespace(aStream)) throw std::runtime_error("Json::Bool : Could not locate value");
        char buffer[5];
        aStream.read(buffer, 4);
        switch(buffer[0]){
        case 't':
            if(std::memcmp(buffer, "true", 4) != 0) throw std::runtime_error("Json::Bool : Value must be either 'true' or 'false'");
            return GenericValue(true);
        case 'f':
            aStream >> buffer[4];
            if(std::memcmp(buffer, "false", 5) != 0) throw std::runtime_error("Json::Bool : Value must be either 'true' or 'false'");
            return GenericValue(false);
        default:
           throw std::runtime_error("Json::Bool : Failed to read value");
        }
    }

    static GenericValue readDouble(IStream& aStream) throw() {
        if(! skipWhitespace(aStream)) throw std::runtime_error("Json::Number : Could not locate value");
        CString buffer;
        char c;
        aStream >> c;
        while((c >= '0' && c <= '9') || c == '-' || c == '.' || c == 'e' || c == 'E') {
            aStream >> c;
        }
        aStream.setOffset(aStream.getOffset() - 1);

        return static_cast<int32_t>(buffer); //! \todo readDouble
    }

    static GenericValue readString(IStream& aStream) {
        bool escaped = false;
        char c;
        aStream >> c;
        if(c != '"') throw std::runtime_error("Json::String : String must be opened by '\"'");
        GenericValue value;
        String<char>& string = value.setString();

        while(! aStream.end()){
            aStream >> c;
            switch(c) {
            case '\\':
                if(escaped){
                    string.pushBack('\\');
                    escaped = false;
                }else{
                    escaped = true;
                }
                break;
            case '"':
                if(escaped){
                    string.pushBack('"');
                    escaped = false;
                }else{
                    return value;
                }
                break;
            default:
                string.pushBack(c);
                escaped = false;
                break;
            }
        }

        throw std::runtime_error("Json::String : String must be closed by '\"'");
    }

    static GenericValue readArray(IStream& aStream) {
        char c;
        aStream >> c;
        if(c != '[') throw std::runtime_error("Json::Array : Array must be opened '['");

        GenericValue value;
        GenericArray& array_ = value.setArray();

        // Check for close
        if(! skipWhitespace(aStream)) throw std::runtime_error("Json::Array : Failed to locate first element");
        if(aStream.peek<char>() == ']') return value;

        while(! aStream.end()) {
            // Read value
            array_.pushBack(readValue(aStream));

            // Read seperator
            if(! skipWhitespace(aStream)) throw std::runtime_error("Json::Array : Failed to locate next element");
            aStream >> c;
            switch(c){
            case ']':
                return value;
            case ',':
                break;
            default:
                #ifdef SOLAIRE_JSON_WHITESPACE_SEPERATORS
                    aStream.setOffset(aStream.getOffset() -1);
                    break;
                #else
                    throw std::runtime_error("Json::Array : Array elements must be separated by ','");
                #endif
            }
        }
        throw std::runtime_error("Json::Array : Array must be closed by ']'");
    }

    static GenericValue readObject(IStream& aStream) {
        char c;
        aStream >> c;
        if(c != '{') throw std::runtime_error("Json::Object : Object must be opened by '{'");

        GenericValue value;
        GenericObject& object = value.setObject();

        // Check for close
        if(! skipWhitespace(aStream)) throw std::runtime_error("Json::Object : Failed to read first member");
        if(aStream.peek<char>() == '}') return value;

        while(! aStream.end()) {
            // Read name
            const GenericValue name = readString(aStream);
            if(! name.isString()) throw std::runtime_error("Json::Object : Member name must be a string");

            // Read divider
            if(! skipWhitespace(aStream)) throw std::runtime_error("Json::Object : Failed to determine position of member name divider");
            aStream >> c;
            if(c != ':') throw std::runtime_error("Json::Object : Member name and value must be separated by ':'");

            // Read value
            object.emplace(name.getString(), readValue(aStream));

            // Read seperator
            if(! skipWhitespace(aStream)) throw std::runtime_error("Json::Object : Failed to determine position of member separator");
            aStream >> c;
            switch(c){
            case '}':
                return value;
            case ',':
                break;
            default:
                #ifdef SOLAIRE_JSON_WHITESPACE_SEPERATORS
                    aStream.setOffset(aStream.getOffset() -1);
                    break;
                #else
                    throw std::runtime_error("Json::Object : Members must be separated by ','");
                #endif
            }
        }
        throw std::runtime_error("Json::Object : Object must be closed by '}'");
    }

    static GenericValue readValue(IStream& aStream) {
        switch(getType(aStream)) {
        case GenericValue::NULL_T:
            return readNull(aStream);
        case GenericValue::BOOL_T:
            return readBool(aStream);
        case GenericValue::DOUBLE_T:
            return readDouble(aStream);
        case GenericValue::STRING_T:
            return readString(aStream);
        case GenericValue::ARRAY_T:
            return readArray(aStream);
        case GenericValue::OBJECT_T:
            return readObject(aStream);
        default:
        throw std::runtime_error("Invalid json type");
        }
    }


    // JsonFormat

    SOLAIRE_EXPORT_CALL JsonFormat::~JsonFormat() {

    }

    GenericValue SOLAIRE_EXPORT_CALL JsonFormat::readValue(IStream& aStream) const throw() {
        try{
            return Solaire::readValue(aStream);
        }catch(std::exception& e) {
            std::cerr << e.what() << std::endl;
            return GenericValue();
        }
    }

    bool SOLAIRE_EXPORT_CALL JsonFormat::writeValue(const GenericValue& aValue, OStream& aStream) const throw() {

        try{
            return Solaire::writeValue(aValue, aStream);
        }catch(std::exception& e) {
            std::cerr << e.what() << std::endl;
            return false;
        }
    }

}
