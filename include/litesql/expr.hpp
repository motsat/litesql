/* LiteSQL 
 * 
 * By Tero Laitinen 
 * 
 * See LICENSE for copyright information. */

#ifndef litesql_expr_hpp
#define litesql_expr_hpp
#include <string>
#include <utility>
#include "litesql/utils.hpp"
#include "litesql/field.hpp"
/** \file expr.hpp 
    Contains Expr-class hierarchy and operator overloadings for them.*/
namespace litesql {
using namespace std;
/** A base class for expression in WHERE - clause. 
    See \ref usage_selecting_persistents */
class Expr {
protected:
    // extra tables to be joined
    Split extraTables;
public:
  /// constant for True expression
  static const char* True; 
    // default expression is true
    virtual string asString() const { return True; }


    Split getExtraTables() const { 
        return extraTables;
    }
    virtual ~Expr() {}
};
/** used to inject custom expression into WHERE-clause */
class RawExpr : public Expr {
    string expr;
public:
    RawExpr(string e) : expr(e) {}
    virtual string asString() const { return expr; }
};
/** used to connect two expressions */
class Connective : public Expr {
private:
    string op;
protected:
    string e1_string;
    string e2_string;
    //const Expr &e1, &e2;
    
    Connective(string o, const Expr & e1_, const Expr & e2_)
        : op(o), e1_string(e1_.asString()), e2_string(e2_.asString()) {
        }

public:        
    virtual ~Connective() {}
    
    virtual string asString() const {
        string res = "(" + e1_string + ") " + op
            + " (" + e2_string + ")";
        return res;
    }
};  
/** connects two expressions with and-operator. */
class And : public Connective {
public:
    And(const Expr & e1_, const Expr & e2_) : Connective("and", e1_, e2_) {}
    virtual string asString() const {     
        if (e1_string == True)
            return e2_string;
        else if (e2_string == True)
            return e1_string;
        else
            return Connective::asString();
    }
};
/** connects two expression with or-operator. */
class Or : public Connective {
public:
    Or(const Expr & e1_, const Expr & e2_) 
        : Connective("or", e1_, e2_) {}
    virtual string asString() const {
        if ((e1_string == True)||(e2_string == True))
            return True;
        else
            return Connective::asString();
    }
};
/** negates expression */
class Not : public Expr {
private:
    const Expr & exp;
    const string exp_string;
public:
    Not(const Expr & _exp) : exp(_exp), exp_string(_exp.asString()) {}
    virtual string asString() const {    
        return "not ("+exp_string+")";
    }
        
};
/** base class of operators in expressions */
class Oper : public Expr {
protected:
    const FieldType & field;
    string op;
    string data;
    bool escape;
    
    Oper(const FieldType & fld, const string& o, const string& d) 
        : field(fld), op(o), data(d), escape(true) {
        extraTables.push_back(fld.table());
    }
    Oper(const FieldType & fld, const string& o, const FieldType &f2) 
        : field(fld), op(o), data(f2.fullName()), escape(false) {
        extraTables.push_back(fld.table());
    }

public:
    virtual string asString() const {
        string res;
        res += field.fullName() + " " + op + " " + (escape ? escapeSQL(data) : data);
        return res;
    }
};
/** equality operator */
class Eq : public Oper {
public:
    Eq(const FieldType & fld, const string& d)
        : Oper(fld, "=", d) {}
    Eq(const FieldType & fld, const FieldType & f2)
        : Oper(fld, "=", f2) {}

};
/** inequality operator */
class NotEq : public Oper {
public:
    NotEq(const FieldType & fld, const string& d)
        : Oper(fld, "<>", d) {}
    NotEq(const FieldType & fld, const FieldType & f2)
        : Oper(fld, "<>", f2) {
    }
   
};
/** greater than operator */
class Gt : public Oper {
public:
    Gt(const FieldType & fld, const string& d)
        : Oper(fld, ">", d) {}
    Gt(const FieldType & fld, const FieldType& d)
        : Oper(fld, ">", d) {}

};
/** greater or equal operator */
class GtEq : public Oper {
public:
    GtEq(const FieldType & fld, const string& d)
        : Oper(fld, ">=", d) {}
    GtEq(const FieldType & fld, const FieldType& d)
        : Oper(fld, ">=", d) {}

};
/** less than operator */
class Lt : public Oper {
public:
    Lt(const FieldType & fld, const string& d)
        : Oper(fld, "<", d) {}
    Lt(const FieldType & fld, const FieldType& d)
        : Oper(fld, "<", d) {}

};
/** less than or equal operator */
class LtEq : public Oper {
public:
    LtEq(const FieldType & fld, const string& d)
        : Oper(fld, "<=", d) {}
    LtEq(const FieldType & fld, const FieldType& d)
        : Oper(fld, "<=", d) {}

};
/** like operator */
class Like : public Oper {
public:
    Like(const FieldType & fld, const string& d)
        : Oper(fld, "like", d) {}
};
class SelectQuery;
/** in operator */
class In : public Oper {
public:
    In(const FieldType & fld, const string& set)
        : Oper(fld, "in", "("+set+")") {};
    In(const FieldType & fld, const SelectQuery& s);
    virtual string asString() const {
        return field.fullName() + " " + op + " " + data;
    }
    
};
And operator&&(const Expr& o1, const Expr& o2);
Or operator||(const Expr& o1, const Expr& o2);
template <class T>
litesql::Eq operator==(const litesql::FieldType& fld, const T& o2) {
    return litesql::Eq(fld, litesql::toString(o2));
}
Eq operator==(const FieldType& fld, const FieldType& f2);
Gt operator>(const FieldType& fld, const FieldType& o2);
GtEq operator>=(const FieldType& fld, const FieldType& o2);
Lt operator<(const FieldType& fld, const FieldType& o2);
LtEq operator<=(const FieldType& fld, const FieldType& o2);
NotEq operator!=(const FieldType& fld, const FieldType& f2);

template <class T>
litesql::Gt operator>(const litesql::FieldType& fld, const T& o2) {
    return litesql::Gt(fld, litesql::toString(o2));
}

template <class T>
litesql::GtEq operator>=(const litesql::FieldType& fld, const T& o2) {
    return litesql::GtEq(fld, litesql::toString(o2));
}

template <class T>
litesql::Lt operator<(const litesql::FieldType& fld, const T& o2) {
    return litesql::Lt(fld, litesql::toString(o2));
}


template <class T>
litesql::LtEq operator<=(const litesql::FieldType& fld, const T& o2) {
    return litesql::LtEq(fld, litesql::toString(o2));
}
template <class T>
litesql::NotEq operator!=(const litesql::FieldType& fld, const T& o2) {
    return litesql::NotEq(fld, litesql::toString(o2));
}

Not operator!(const Expr &exp);
}


#endif
