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
        const auto end = aValue.end();
        for(auto i = aValue.begin(); i != end; ++i) if(! writeValue(*i, aStream)) return false;
        aStream << ']';
        return true;
    }

    static bool writeObject(const GenericObject& aValue, OStream& aStream) throw() {
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


    // JsonFormat

    SOLAIRE_EXPORT_CALL JsonFormat::~JsonFormat() {

    }

    GenericValue SOLAIRE_EXPORT_CALL JsonFormat::readValue(IStream&) const throw() {

    }

    bool SOLAIRE_EXPORT_CALL JsonFormat::writeValue(const GenericValue& aValue, OStream& aStream) const throw() {
        return Solaire::writeValue(aValue, aStream);
    }

}
