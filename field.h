/* Copyright 2011 ZAO "Begun".
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __SLAVE_FIELD_H_
#define __SLAVE_FIELD_H_

#include <string>
#include <vector>
#include <list>

#include "collate.h"
#include "types.h"

// conflict with macro defined in mysql
#ifdef test
#undef test
#endif /* test */

namespace slave
{

class Field
{
public:
    const std::string field_type;
    const std::string field_name;

    FieldValue field_data;

    virtual const char* unpack(const char *from) = 0;

    Field(const std::string& field_name_arg, const std::string& type) :
        field_type(type),
        field_name(field_name_arg)
        {}

    virtual ~Field() {}

    virtual unsigned int pack_length() const = 0;

    const std::string getFieldName() {
        return field_name;
    }
};

class Field_num: public Field {
public:
    Field_num(const std::string& field_name_arg, const std::string& type);
};


class Field_str: public Field {
public:
    Field_str(const std::string& field_name_arg, const std::string& type);
};

class Field_longstr: public Field_str {

protected:
    unsigned int field_length;

public:
    Field_longstr(const std::string& field_name_arg, const std::string& type);

    unsigned int pack_length() const {
        return field_length;
    }
};


class Field_real: public Field_num {
public:
    Field_real(const std::string& field_name_arg, const std::string& type);
};

class Field_tiny: public Field_num {
    unsigned int pack_length() const { return 1; }
public:
    Field_tiny(const std::string& field_name_arg, const std::string& type);
    const char* unpack(const char* from);
};

class Field_short: public Field_num {
    unsigned int pack_length() const { return 2; }
public:
    Field_short(const std::string& field_name_arg, const std::string& type);

    const char* unpack(const char* from);
};

class Field_medium: public Field_num {
    unsigned int pack_length() const { return 3; }
public:
    Field_medium(const std::string& field_name_arg, const std::string& type);

    const char* unpack(const char* from);
};

class Field_long: public Field_num {
    unsigned int pack_length() const { return 4; }
public:
    Field_long(const std::string& field_name_arg, const std::string& type);

    const char* unpack(const char* from);
};

class Field_longlong: public Field_num {
    unsigned int pack_length() const { return 8; }
public:
    Field_longlong(const std::string& field_name_arg, const std::string& type);

    const char* unpack(const char* from);
};

class Field_float: public Field_real {
    unsigned int pack_length() const { return sizeof(float); }
public:
    Field_float(const std::string& field_name_arg, const std::string& type);

    const char* unpack(const char* from);
};

class Field_double: public Field_real {
    unsigned int pack_length() const { return sizeof(double); }
public:
    Field_double(const std::string& field_name_arg, const std::string& type);

    const char* unpack(const char* from);
};

class Field_temporal: public Field_longstr {
protected:
    bool is_old_storage;
public:
    Field_temporal(const std::string& field_name_arg, const std::string& type, bool old_storage);
    virtual ~Field_temporal() {}

    virtual void reset(bool old_storage, bool ctor_call = false) = 0;
};

class Field_timestamp: public Field_temporal {
public:
    Field_timestamp(const std::string& field_name_arg, const std::string& type, bool old_storage);

    void reset(bool old_storage, bool ctor_call = false);
    const char* unpack(const char* from);
};

class Field_year: public Field_tiny {
public:
    Field_year(const std::string& field_name_arg, const std::string& type);
};

class Field_date: public Field_str {
    unsigned int pack_length() const { return 3; }
public:
    Field_date(const std::string& field_name_arg, const std::string& type);

    const char* unpack(const char* from);
};

class Field_time: public Field_temporal {
public:
    Field_time(const std::string& field_name_arg, const std::string& type, bool old_storage);

    void reset(bool old_storage, bool ctor_call = false);
    const char* unpack(const char* from);
};

class Field_datetime: public Field_temporal {
public:
    Field_datetime(const std::string& field_name_arg, const std::string& type, bool old_storage);

    void reset(bool old_storage, bool ctor_call = false);
    const char* unpack(const char* from);
};

class Field_varstring: public Field_longstr {

    // How many bytes are needed for holding the length
    unsigned int length_bytes;

    unsigned int pack_length() const { return (unsigned int) field_length+length_bytes; }

public:
    Field_varstring(const std::string& field_name_arg, const std::string& type,
                    const collate_info& collate);

    const char* unpack(const char* from);
};

class Field_blob: public Field_longstr {
    unsigned int get_length(const char *ptr);
public:
    Field_blob(const std::string& field_name_arg, const std::string& type);

    const char* unpack(const char* from);

protected:
    // Number of bytes for holding the data length
    unsigned int packlength;
};

class Field_tinyblob: public Field_blob {
public:
    Field_tinyblob(const std::string& field_name_arg, const std::string& type);
};

class Field_mediumblob: public Field_blob {
public:
    Field_mediumblob(const std::string& field_name_arg, const std::string& type);
};

class Field_longblob: public Field_blob {
public:
    Field_longblob(const std::string& field_name_arg, const std::string& type);
};

class Field_enum: public Field_str {

    unsigned int pack_length() const {
        return (unsigned int)(count_elements < 255) ? 1 : 2;
    }

public:
    Field_enum(const std::string& field_name_arg, const std::string& type);


    const char* unpack(const char* from);

protected:
    unsigned int packlength;

    // Number of elements in enum
    unsigned short count_elements;
};

class Field_set: public Field_enum {

    unsigned int pack_length() const {
        unsigned int x = (unsigned int) ((count_elements + 7)/8);
        x = (x > 4 ? 8 : x);
        return x;
    }

public:
    Field_set(const std::string& field_name_arg, const std::string& type);

    const char* unpack(const char* from);
};

class Field_decimal : public Field_longstr {
    double dec2double(const char*);
    int intg;
    int frac;
public:
    Field_decimal(const std::string& field_name_arg, const std::string& type);
    const char* unpack(const char *from);
};

class Field_bit : public Field
{
    unsigned int _pack_length;

public:
    Field_bit(const std::string& field_name_arg, const std::string& type);

    const char* unpack(const char *from);

    unsigned int pack_length() const {
        return _pack_length;
    }
};


}

#endif
