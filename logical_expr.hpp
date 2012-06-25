#ifndef LOGICAL_EXPRESSION
#define LOGICAL_EXPRESSION


#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/dynamic_bitset.hpp>


namespace logical_expr {


using namespace std;

// Don't care
static const boost::optional<bool> dont_care = boost::none;

// expr_mode is not used now. 
// But this will be used to change the expression of functions
enum expr_mode { alphabet_expr };

/*
 * String to be parsed has to be in the following form:
 * ${Function-Name}(Variables-divided-by-',' ...) = ${TERMS} + ...
 * White spaces will be ignored
 * Default character to invert a variable is '^' (first template parameter)
 */
template<char inverter = '^', bool escape = true, expr_mode mode = alphabet_expr>
class function_parser {
public:
    typedef std::pair<string, vector<string>> result_type;
    function_parser() {}
    explicit function_parser(const string &expr) : expr_(expr) {}
    ~function_parser() {}

    void set_expression(const string &expr) { expr_ = expr; }
    string set_expression() { return expr_; }
    const string& function_name() const { return func_name_; }

    result_type parse() {
        auto untokenized = scanner();
        auto token = tokenizer(untokenized);
        used_undeclared_vars(token.second, token.first);
        return std::move(token);
    }

    vector<string> scanner() {
        if( expr_.empty() )
            throw std::runtime_error("expr: Expression is empty, aborted");
        string expr_with_nospaces = boost::regex_replace(expr_, boost::regex("\\s"), "");
        boost::regex reg(
            (boost::format("([A-Za-z_-]+)\\((((\\s*[A-Z],)*)([A-Z]))\\)=(((%1%?[A-Z])+\\+)*((%1%?[A-Z])+))$") 
                % (escape ? string{'\\', inverter} : string{inverter})).str(),
             boost::regex::perl
        );
        boost::smatch result;
        if( !boost::regex_match(expr_with_nospaces, result, reg) )
            throw std::runtime_error("expr: Input string does not match the correct form");
        func_name_ = result[1];
//        for( unsigned int i = 0; i < result.size(); ++i )
//            cout << "result[" << i << "] = " << result[i] << endl;
//                            decl-vars  decl-terms
        return vector<string>{result[2], result[6]};
    }

    static result_type tokenizer(const vector<string> &untokenized) {
        std::vector<string> vars, terms;
        typedef boost::char_separator<char> char_separator;
        boost::tokenizer<char_separator> 
            var_tokenizer(untokenized[0], char_separator(",")), term_tokenizer(untokenized[1], char_separator("+"));
        ostringstream oss;
        for( auto token : var_tokenizer )   oss << token;
        for( auto token : term_tokenizer )  terms.push_back(token);
        return std::make_pair(oss.str(), terms);
    }

    static bool used_undeclared_vars(const vector<string> &terms, const string &vars) {
        for( auto term : terms ) {
          for( auto used_var : term ) {
            if( used_var != inverter && vars.find(used_var, 0) == string::npos )
              throw std::runtime_error(
                (boost::format("expr: Using undeclared variable, %c") % used_var).str()
                );
          }
        }
        return true;
    }

private:
    string expr_, func_name_;
};


class logical_term {
public:
    typedef boost::optional<bool> value_type;    
    typedef boost::dynamic_bitset<> arg_type;
    typedef logical_term this_type;
    
    logical_term() : inverter_('^') {}
    logical_term(const string &expr, int bitsize, char inverter = '^') 
        : expr_(expr), inverter_(inverter), term_(bitsize, logical_expr::dont_care) { parse(expr_); }

    int size() const { return term_.size(); }
    const string& get_expr() { return expr_; }

    bool size_check(const arg_type &arg) const { return (size() == arg.size()); }
    bool size_check(const this_type &term) const { return (size() == term.size()); }
    static bool size_check(const this_type &first, const this_type &second) { return (first.size() == second.size()); }
    static bool size_check(const arg_type &first, const arg_type &second) { return (first.size() == second.size()); }

    bool calculate(const arg_type &arg) const {
        if( !size_check(arg) )
            throw std::runtime_error("target two operands are not same size");
        bool ret = true;
        for( int i = 0; i < term_.size(); ++i )
            ret = ret && (term_[i] == dont_care ? true : arg[term_.size() - 1 -  i] == term_[i]);
        return ret;
    }

    bool operator()(const arg_type &arg) const { 
        return calculate(arg);
    }

    value_type& operator[](int index) { return term_[index]; }
    bool operator==(const logical_term &term) {
        if( !size_check(term) ) return false;
        for( int i = 0; i < std::pow(2, size()); ++i ) {
            logical_expr::logical_term::arg_type arg(size(), i);
            if( calculate(arg) != term.calculate(arg) )
                return false;
        }
        return true;
    }

    friend ostream& operator<<(ostream &os, const logical_term &bf) {
        for( auto b : bf.term_ ) {
            if( b ) os << *b;
            else    os << 'x';
        }
        return os;
    }

private:
    void parse(const string &expr) {
        bool invert = false;
        for( char var : expr ) {
            if( var == inverter_ ) {
                invert = true;
                continue;
            }
            term_[var - 'A'] = !invert;
            invert = false;
        }
    }
    
    string expr_;
    char inverter_;
    vector<value_type> term_;
};


class logical_function {
public:
    typedef logical_term value_type;
    typedef boost::dynamic_bitset<> arg_type;
    typedef vector<value_type>::iterator iterator;
    typedef vector<value_type>::const_iterator const_iterator;

    logical_function() {}
    explicit logical_function(const logical_term &term) {  }
    ~logical_function() {}

    iterator begin() { return func_.begin(); }
    const_iterator begin() const { return func_.begin(); }
    iterator end() { return func_.end(); }
    const_iterator end() const { return func_.end(); }

    int size() const { return func_.size(); }
    int term_size() const {
        if( size() == 0 ) return 0;
        return func_[0].size();
    }
    void add(const logical_term &term) { func_.push_back(term); }
    void add(const logical_function &func) {
        for( auto term : func )
            add(term);
    }
    
    bool calculate(const arg_type &arg) const {
        bool ret = false;
        for( auto term : func_ )
            ret = ret || term(arg);
        return ret;
    }

    bool operator()(const arg_type &arg) const { return calculate(arg); }
    value_type& operator[](int index) { return func_[index]; }

    logical_function operator+(const logical_term &term) {
        
        logical_function ret(*this);
        ret += term;
        return ret;
    }
    logical_function  operator+(const logical_function &func) {
        logical_function ret(*this);
        ret += func;
        return ret;
    }
    logical_function& operator+=(const logical_term &term) { add(term); return *this; }
    logical_function& operator+=(const logical_function &func) { add(func); return *this; }
    friend ostream& operator<<(ostream &os, const logical_function &bf) {
        for( auto term : bf.func_ )
            os << term << " ";
        return os;
    }

private:
    vector<logical_term> func_;
};

logical_function operator+(const logical_term &first, const logical_term &second) {
    logical_function ret(first);
    ret += second;
    return ret;
}


}   // namespace logical_expr


#endif  // LOGICAL_EXPRESSION
