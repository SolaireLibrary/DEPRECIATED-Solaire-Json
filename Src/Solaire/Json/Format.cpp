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
        return false;
    }

    static bool writeUnsigned(const uint64_t aValue, OStream& aStream) throw() {
        return false;
    }

    static bool writeSigned(const int64_t aValue, OStream& aStream) throw() {
        return false;
    }

    static bool writeDouble(const double aValue, OStream& aStream) throw() {
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
        for(const GenericValue& i : aValue) if(! writeValue(i, aStream)) return false;
        aStream << ']';
        return true;
    }

    static bool writeObject(const GenericObject& aValue, OStream& aStream) throw() {
        aStream << '{';
        const SharedAllocation<StaticContainer<GenericObject::Entry>> entries = aValue.getEntries();
        for(const GenericObject::Entry& i : *entries) {
            if(! writeString(i.first, aStream)) return false;
            aStream << ':';
            if(! writeValue(i.second, aStream)) return false;
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

    static GenericValue readValue(IStream& aStream) throw();

    static GenericValue readNull(IStream& aStream) throw() {
        if(! skipWhitespace(aStream)) GenericValue();
        char buffer[4];
        aStream.read(buffer, 4); // read "null"
        return GenericValue();
    }

    static GenericValue readBool(IStream& aStream) throw() {
        if(! skipWhitespace(aStream)) GenericValue();
        char buffer[4];
        aStream.read(buffer, 4); // read "true"
        switch(buffer[0]){
        case 't':
            return GenericValue(true);
        case 'f':
            aStream.read(buffer, 1); // read "false"
            return GenericValue(false);
        default:
            return GenericValue();
        }
    }

    static GenericValue readDouble(IStream& aStream) throw() {
        return GenericValue();
    }

    static GenericValue readString(IStream& aStream) throw() {
        bool escaped = false;
        char c;
        aStream >> c; // Skip opening '"'
        if(c != '"') return GenericValue();
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

        return GenericValue();
    }

    static GenericValue readArray(IStream& aStream) throw() {
        return GenericValue();
    }

    static GenericValue readObject(IStream& aStream) throw() {
        return GenericValue();
    }

    static GenericValue readValue(IStream& aStream) throw() {
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
            return readString(aStream);
        case GenericValue::OBJECT_T:
            return readObject(aStream);
        default:
            return GenericValue();
        }
    }


    // JsonFormat

    SOLAIRE_EXPORT_CALL JsonFormat::~JsonFormat() {

    }

    GenericValue SOLAIRE_EXPORT_CALL JsonFormat::readValue(IStream& aStream) const throw() {
        return Solaire::readValue(aStream);
    }

    bool SOLAIRE_EXPORT_CALL JsonFormat::writeValue(const GenericValue& aValue, OStream& aStream) const throw() {
        return Solaire::writeValue(aValue, aStream);
    }

}
