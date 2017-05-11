#ifndef LOGICAL_EXPRESSION_HPP
#define LOGICAL_EXPRESSION_HPP


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
#include <boost/call_traits.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/optional.hpp>
//#include <boost/logic/tribool.hpp>

//
// namespace for Logical Expression
//
namespace logical_expr {


using namespace std;

// Don't care
/*thread_local*/static const boost::optional<bool> dont_care = boost::none;
// static const boost::logic::tribool dont_care = indeterminated; better than optional<bool>

// expr_mode is not used now. 
enum expr_mode { alphabet_expr, verilog_expr, truth_table };

// argument generating iterator for logical_function
class arg_gen_iterator {
public:
    typedef boost::dynamic_bitset<> value_type;
    typedef arg_gen_iterator this_type;
    arg_gen_iterator(int width, int val) 
        : width_(width), current_val_(val), value_(width, val) {}
    this_type& operator++() {
        value_type tmp(width_, current_val_ + 1);
        value_.swap(tmp);   // never throw any exceptions
        ++current_val_;
        return *this;
    }
    this_type operator++(int)
        { this_type before = *this; ++*this; return before; }
    this_type& operator--() {
        value_type tmp(width_, current_val_ - 1);
        value_.swap(tmp);
        --current_val_;
        return *this;
    }
    this_type operator--(int)
        { this_type before = *this; --*this; return before; }
    bool operator<(const this_type &it) const
        { return (it.width_ == width_ && current_val_ < it.current_val_); }
    bool operator>(const this_type &it) const
        { return (it.width_ == width_ && current_val_ > it.current_val_); }
    bool operator==(const this_type &it) const 
        { return (it.width_ == width_ && current_val_ == it.current_val_); }
    bool operator!=(const this_type &it) const 
        { return !(*this == it); }
    const value_type& operator*() const { return value_; }
private:
    const int width_;
    int current_val_;
    value_type value_;
};

// Argument generator for logical_function using Iterator (default: arg_gen_iterator)
template<typename Iterator = arg_gen_iterator>
class arg_generator {
public:
    arg_generator(int nbegin, int nend, int width) 
        : begin_(width, nbegin), end_(width, nend) {}
    const Iterator& begin() const { return begin_; }
    const Iterator&  end()  const { return end_; }
private:
    const Iterator begin_, end_;
};


//
// * String to be parsed has to be in the following form:
// * ${Function-Name}(Variables-divided-by-',' ...) = ${TERMS} + ...
// * White spaces will be ignored
// * Default character to invert a variable is '~' (first template parameter)
// See README for more information about parsing
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
template<char inverter = '~', bool escape = true, expr_mode mode = alphabet_expr>
class function_parser {
public:
    typedef std::pair<string, vector<string>> result_type;
    function_parser() {}
    explicit function_parser(const string &expr, const char first) : expr_(expr), first_char_(first) {}
    ~function_parser() {}

    void set_expression(const string &expr) { expr_ = expr; }
    const string& get_expression() const { return expr_; }
    const string& function_name() const { return func_name_; }

    result_type parse() {
        auto untokenized = scanner();
        auto token = tokenizer(untokenized);
        boost::optional<char> undecl = use_undeclared_vars(token.second, token.first);
        if( undecl )
            throw std::runtime_error(
                (boost::format("expr: Using undeclared variable, %c") % *undecl).str()
            );
        if( !is_sequence(token.first, first_char_) )   
            throw std::runtime_error("expr: used variables are not sequence");
        return std::move(token);
    }

    vector<string> scanner() {
        if( expr_.empty() )
            throw std::runtime_error("expr: Expression is empty, aborted");
        string expr_with_nospaces = boost::regex_replace(expr_, boost::regex("\\s"), "");
        boost::regex reg(
            (boost::format("([A-Za-z_-]+)\\((((\\s*[A-Za-z],)*)([A-Za-z]))\\)=(((%1%?[A-Za-z])+\\+)*((%1%?[A-Za-z])+))$") 
                % (escape ? string{'\\', inverter} : string{inverter})).str(),
             boost::regex::perl
        );
        boost::smatch result;
        if( !boost::regex_match(expr_with_nospaces, result, reg) )
            throw std::runtime_error("expr: Input string does not match the correct form");
        func_name_ = result[1];
//                            decl-vars  decl-terms
        return vector<string>{result[2], result[6]};
    }

    static result_type tokenizer(const vector<string> &untokenized) {
        std::vector<string> terms;
        typedef boost::char_separator<char> char_separator;
        boost::tokenizer<char_separator> 
            var_tokenizer(untokenized[0], char_separator(",")), term_tokenizer(untokenized[1], char_separator("+"));
        ostringstream oss;
        for( auto token : var_tokenizer )   oss << token;
        for( auto token : term_tokenizer )  terms.push_back(token);
        return std::make_pair(oss.str(), terms);
    }
    
    static bool is_sequence(const string &vars, char first_char) {
        if( vars[0] != first_char )
            throw std::runtime_error("expr: declare terms which starts with not specified char");
        auto previous = vars.begin();
        for( auto it = ++vars.begin(); it != vars.end(); ++it ) {
            if( *it != static_cast<char>(*previous + 1) )
                return false;
            previous = it;
        }
        return true;
    }

    static boost::optional<char> use_undeclared_vars(const vector<string> &terms, const string &vars) {
        for( auto term : terms )
          for( auto used_var : term )
            if( used_var != inverter && vars.find(used_var, 0) == string::npos )
              return (used_var);
        return boost::none;
    }

private:
    string expr_, func_name_;
    char first_char_;
};


//
// Term properties
//
// Type Requirements:
//  [*] typedef value type
//  [*] Have set() and get() member functions
//  [*] Default constructible
//  [*] Has swap() member fuction that never throws any exceptions for expcetion-safe
//

// Term Property template class for POD types
template<typename T, T DefaultValue>
class term_property_pod {
public:
    typedef T value_type;
    term_property_pod() : value_(DefaultValue) {}
    typename boost::call_traits<T>::param_type get() const { return value_; }
    void set(typename boost::call_traits<T>::param_type value) { value_ = value; }
    void swap(const term_property_pod<T, DefaultValue> &property) noexcept(true)
        { std::swap(value_, property.value_); }
private:
    value_type value_;
};

typedef term_property_pod<int, 0> term_no_property;
typedef term_no_property term_dummy;
typedef term_property_pod<char, ' '> term_name;
typedef term_property_pod<bool, false> term_mark;
typedef term_property_pod<unsigned int, 0> term_number;
//

//
// class: logical term
//
template<typename Property_ = term_dummy>
class logical_term {
public:
    typedef boost::optional<bool> value_type;    
    typedef boost::dynamic_bitset<> arg_type;
    typedef logical_term<Property_> this_type;
    typedef std::size_t size_t;
    typedef Property_ property_type;
    
    logical_term() {}
    logical_term(int bitsize, const value_type &init = logical_expr::dont_care) 
        : term_(bitsize, init) {}
    template<typename Property>
    explicit logical_term(const logical_term<Property> &term) 
        { construct_from(term); }
    explicit logical_term(const arg_type &arg) {
        for( int i = arg.size() - 1; 0 <= i; --i )
            term_.push_back(arg[i]);
    }

    template<typename Property>
    void construct_from(const logical_term<Property> &term) 
        { term_ = term.term_; }

    template<typename Property>
    void swap(logical_term<Property> &term) noexcept(true) {
        term_.swap(term.term_);
        property_.swap(term.property_);
    }

    size_t size() const 
        { return term_.size(); }

    bool size_check(const arg_type &arg) const 
        { return (size() == arg.size()); }

    bool is_same(const this_type &term) const
        { return (term.term_ == term_); }

    template<typename Property>
    bool size_check(const logical_term<Property> &term) const 
        { return ( size() == term.size() ); }

    size_t num_of_value(bool value) const {
        return static_cast<size_t>(std::count_if(term_.begin(), term_.end(), 
                [value](const value_type &b){ return (b != dont_care && *b == value); }));
    }

    size_t diff_size(const this_type &term) const {
        if( !size_check(term) )
            throw std::runtime_error(size_error_msg);
        size_t diff_count = 0;
        for( int i = 0; i < size(); ++i )
            if( term_[i] != term[i] )
                ++diff_count;
        return diff_count;
    }

    bool calculate(const arg_type &arg) const {
        if( !size_check(arg) )
            throw std::runtime_error(size_error_msg);
        bool ret = true;
        for( int i = 0; i < term_.size(); ++i )
            ret = ret && (term_[i] == dont_care ? true : arg[term_.size()-1-i] == term_[i]);
        return ret;
    }

    bool operator()(const arg_type &arg) const 
        { return calculate(arg); }

    value_type& operator[](int index)
        { return term_[index]; }
    const value_type& operator[](int index) const
        { return term_[index]; }
    
    template<typename Property>
    bool operator==(const logical_term<Property> &term) const {
        if( !size_check(term) ) return false;
        arg_generator<> gen(0, std::pow(2, size()), size());
        for( auto it = gen.begin(); it != gen.end(); ++it )
            if( calculate(*it) != term.calculate(*it) )
                return false;
        return true;
    }

    template<typename Property>
    friend typename logical_term<Property>::property_type::value_type
        property_get(const logical_term<Property>& term);

    template<typename Property>
    friend void property_set(logical_term<Property>& term,
        const typename logical_term<Property>::property_type::value_type &arg);

    template<typename Property>
    friend std::ostream& operator<<(std::ostream &os, const logical_expr::logical_term<Property> &bf) {
        boost::io::ios_flags_saver ifs(os);
        for( auto b : bf.term_ ) {
            if( b ) os << std::noboolalpha << *b;
            else    os << 'x';
        }
        return os;
    }

private:
    static const std::string size_error_msg;
    vector<value_type> term_;
    property_type property_;
};
template<typename Property>
const string logical_term<Property>::size_error_msg = "target two operands are not same size";


// Create a logical_term with Property parsed from expr
template<typename Property = term_no_property, char Inverter = '^'>
logical_term<Property>
parse_logical_term(const string &expr, int bitsize, char const first_char = 'A') {
    logical_term<Property> term(bitsize);
    for( auto it = expr.begin(); it != expr.end(); ++it ) {
        bool value = true;
        if( *it == Inverter ) {
            ++it; value = false;
        }
        term[*it - first_char] = value;
    }
    return term;
}

//
// Setter and getter functions of term property
//
template<typename Property>
typename logical_term<Property>::property_type::value_type
    property_get(const logical_term<Property>& term) 
{ return term.property_.get(); }

template<typename Property>
void property_set(logical_term<Property>& term,
    const typename logical_term<Property>::property_type::value_type &arg)
{ term.property_.set(arg); }


//
// Minimize the different 1bit of term a and b 
//
template<typename Property>
logical_term<Property> onebit_minimize(const logical_term<Property> &a, const logical_term<Property> &b)
{
    if( a.size() != b.size() || 1 < a.diff_size(b) )
        throw std::runtime_error("tried to minimize a term which has more than 1bit different bits");
    logical_term<Property> term(a);
    for( int i = 0; i < term.size(); ++i )
        if( term[i] != b[i] )
            term[i] = dont_care;
    return std::move(term);
}

// Return minimized term which has pval as its property value
template<typename Property>
logical_term<Property> onebit_minimize(
        const logical_term<Property> &a, 
        const logical_term<Property> &b, 
        const typename logical_term<Property>::property_type::value_type &pval
    )
{
    logical_term<Property> term = onebit_minimize(a, b);
    property_set(term, pval);
    return std::move(term);
}


//
// class: logical function
//
template<typename TermType>
class logical_function {
public:
    typedef std::size_t size_t;
    typedef TermType value_type;
    typedef boost::dynamic_bitset<> arg_type;
    typedef typename vector<value_type>::iterator iterator;
    typedef typename vector<value_type>::const_iterator const_iterator;
    typedef logical_function<TermType> this_type;

    logical_function() {}
    explicit logical_function(const TermType &term) { add(term); }
    ~logical_function() {}

    iterator begin()             { return func_.begin(); }
    const_iterator begin() const { return func_.begin(); }
    iterator end()               { return func_.end(); }
    const_iterator end()   const { return func_.end(); }

    void swap(logical_function<TermType> &func) noexcept(true)
        { func_.swap(func.func_); }

    int size() const
        { return func_.size(); }
    size_t term_size() const {
        if( func_.empty() ) return 0;
        return func_[0].size();
    }
    void add(const TermType &term)
        { func_.push_back(term); }

    void add(const this_type &func) {
        vector<TermType> tmp(func_);
        for( auto term : func )
            tmp.push_back(term);
        func_ = std::move(tmp);
    }

    void clear()
        { func_.clear(); }
    
    bool calculate(const arg_type &arg) const {
        bool ret = false;
        for( auto term : func_ )
            ret = ret || term(arg);
        return ret;
    }

    bool is_same(const this_type &func) const {
        if( size() != func.size() )
            return false;
        for( const value_type &term : func_ )
            if( std::find_if(func.begin(), func.end(),
                [&](const value_type &t){ return t.is_same(term); })
                    == func.end() )
                return false;
        return true;
    }

    bool operator()(const arg_type &arg) const
        { return calculate(arg); }
    value_type& operator[](int index)
        { return func_[index]; }
    const value_type& operator[](int index) const
        { return func_[index]; }
    
    // The expression like "term + term = func" is not allowed
    const logical_function operator+(const TermType &term) {
        logical_function ret(*this);
        ret += term;
        return ret;
    }

    const logical_function operator+(const logical_function<TermType> &func) {
        logical_function ret(*this);
        ret += func;
        return ret;
    }
    
    logical_function& operator+=(const TermType &term)
        { add(term); return *this; }
    logical_function& operator+=(const logical_function &func)
        { add(func); return *this; }
    friend ostream& operator<<(ostream &os, const logical_function &bf) {
        for( auto term : bf.func_ )
            os << term << " ";
        return os;
    }

    template<typename Property>
    bool operator==(const logical_function<logical_term<Property>> &func) const {
        arg_generator<> gen(0, std::pow(2, func.term_size()), func.term_size());
        for( auto it = gen.begin(); it != gen.end(); ++it )
            if( calculate(*it) != func.calculate(*it) )
                return false;
        return true;
    }

private:
    vector<TermType> func_;
};


}   // namespace logical_expr


template<typename Property>
logical_expr::logical_function<logical_expr::logical_term<Property>> operator+
    (const logical_expr::logical_term<Property> &first, const logical_expr::logical_term<Property> &second) {
    logical_expr::logical_function<logical_expr::logical_term<Property>> ret(first);
    ret += second;
    return ret;
}


#endif  // LOGICAL_EXPRESSION_HPP
