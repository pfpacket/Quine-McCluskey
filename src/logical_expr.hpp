
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

//
// namespace for Logical Expression
//
namespace logical_expr {


using namespace std;

// Don't care
static const boost::optional<bool> dont_care = boost::none;

// expr_mode is not used now. 
// But this will be used to change the expression of functions
enum expr_mode { alphabet_expr };


class arg_gen_iterator {
public:
    typedef boost::dynamic_bitset<> value_type;
    typedef arg_gen_iterator this_type;
    arg_gen_iterator(int width, int val) 
        : width_(width), current_val_(val), value_(width, val) {}
    this_type& operator++() 
        { value_ = value_type(width_, ++current_val_); return *this; }
    this_type  operator++(int) 
        { this_type it = *this; value_ = value_type(width_, ++current_val_); return it; }
    this_type& operator--() 
        { value_ = value_type(width_, --current_val_); return *this; }
    this_type  operator--(int) 
        { this_type it = *this; value_ = value_type(width_, --current_val_); return it; }
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
// * Default character to invert a variable is '^' (first template parameter)
// See README for more information about parsing
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
template<char inverter = '^', bool escape = true, expr_mode mode = alphabet_expr>
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
            (boost::format("([A-Za-z_-]+)\\((((\\s*[A-Z],)*)([A-Z]))\\)=(((%1%?[A-Z])+\\+)*((%1%?[A-Z])+))$") 
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
        std::vector<string> vars, terms;
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
        boost::optional<char> ret = boost::none;
        for( auto term : terms )
          for( auto used_var : term )
            if( used_var != inverter && vars.find(used_var, 0) == string::npos )
              return (ret = used_var);
        return ret;
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
//

// Term Property template class for POD types
template<typename T, T DefaultValue>
class term_property_pod {
public:
    typedef T value_type;
    term_property_pod() : value_(DefaultValue) {}
    typename boost::call_traits<T>::param_type get() const { return value_; }
    void set(typename boost::call_traits<T>::param_type value) { value_ = value; }
private:
    value_type value_;
};

typedef term_property_pod<int, 0> term_no_property;
typedef term_no_property term_dummy;
typedef term_property_pod<char, ' '> term_name;
typedef term_property_pod<bool, false> term_mark;
typedef term_property_pod<unsigned int, 0> term_number;
//

template<typename Property_ = term_dummy>
class logical_term {
public:
    typedef boost::optional<bool> value_type;    
    typedef boost::dynamic_bitset<> arg_type;
    typedef logical_term<Property_> this_type;
    typedef Property_ property_type;
    
    logical_term() {}
    logical_term(int bitsize, const value_type &init = logical_expr::dont_care) 
        : term_(bitsize, init) {}
    explicit logical_term(const arg_type &arg) {
        for( int i = arg.size() - 1; 0 <= i; --i )
            term_.push_back(arg[i]);
    }

    template<typename Prop, char Inv>
    void build_from(const this_type &term) { term_ = term.get_term(); }
    unsigned int size() const { return term_.size(); }
    const vector<value_type>& get_term() const { return term_; }
    vector<value_type>& set_term() const { return term_; }

    bool size_check(const arg_type &arg) const { return (size() == arg.size()); }

    template<typename Property>
    bool size_check(const logical_term<Property> &term) const { return (size() == term.size()); }

    int num_of_value(bool value) const {
        int value_count = 0;
        for( auto b : term_ )
            if( b != dont_care && *b == value )
                ++value_count;
        return value_count;
    }

    int diff_size(const this_type &term) const {
        if( !size_check(term) ) return -1;
        int diff_count = 0;
        for( int i = 0; i < size(); ++i )
            if( term_[i] != term[i] )
                ++diff_count;
        return diff_count;
    }

    bool calculate(const arg_type &arg) const {
        if( !size_check(arg) )
            throw std::runtime_error("target two operands are not same size");
        bool ret = true;
        for( int i = 0; i < term_.size(); ++i )
            ret = ret && (term_[i] == dont_care ? true : arg[term_.size()-1-i] == term_[i]);
        return ret;
    }

    bool is_same(const this_type &term) const { return (term.term_ == term_); }

    bool operator()(const arg_type &arg) const {
        return calculate(arg);
    }

    value_type& operator[](int index) { return term_[index]; }
    const value_type& operator[](int index) const { return term_[index]; }
    
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


private:
    vector<value_type> term_;
    property_type property_;
};


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
    return std::move(term);
}


template<typename Property>
typename logical_term<Property>::property_type::value_type
    property_get(const logical_term<Property>& term) 
{ return term.property_.get(); }

template<typename Property>
void property_set(logical_term<Property>& term,
    const typename logical_term<Property>::property_type::value_type &arg)
{ term.property_.set(arg); }

template<typename Property>
logical_term<Property> onebit_minimize(const logical_term<Property> &a, const logical_term<Property> &b)
{
    if( a.size() != b.size() || 1 < a.diff_size(b) )
        throw std::runtime_error("tried to minimize two different size terms");
    logical_term<Property> term(a);
    for( int i = 0; i < term.size(); ++i )
        if( term[i] != b[i] )
            term[i] = dont_care;
    return std::move(term);
}

template<typename Property>
logical_term<Property> onebit_minimize(const logical_term<Property> &a, 
        const logical_term<Property> &b, const typename logical_term<Property>::property_type::value_type &pval)
{
    logical_term<Property> term = onebit_minimize(a, b);
    property_set(term, pval);
    return std::move(term);
}


}   // namespace logical_expr

template<typename Property>
std::ostream& operator<<(std::ostream &os, const logical_expr::logical_term<Property> &bf) {
        for( auto b : bf.get_term() ) {
            if( b ) os << *b;
            else    os << 'x';
        }
        return os;
}


namespace logical_expr {


template<typename TermType>
class logical_function {
public:
    typedef TermType value_type;
    typedef boost::dynamic_bitset<> arg_type;
    typedef typename vector<value_type>::iterator iterator;
    typedef typename vector<value_type>::const_iterator const_iterator;
    typedef int id_type;
    typedef logical_function<TermType> this_type;

    logical_function() {}
    explicit logical_function(const TermType &term) { add(term); }
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
    void add(const TermType &term) { func_.push_back(term); }
    void add(const this_type &func) {
        for( auto term : func )
            add(term);
    }

    void clear() { func_.clear(); }
    
    bool calculate(const arg_type &arg) const {
        bool ret = false;
        for( auto term : func_ )
            ret = ret || term(arg);
        return ret;
    }

    bool is_same(const this_type &func) const {
        for( int i = 0; i < size(); ++i ) {
            bool find_same = false;
            for( int j = 0; j < size(); ++j )
                find_same |= (*this)[i].is_same(func[j]);
            if( !find_same ) return false;
        }
        return true;
    }

    bool operator()(const arg_type &arg) const { return calculate(arg); }
    value_type& operator[](int index) { return func_[index]; }
    const value_type&  operator[](int index) const { return func_[index]; }
    
    logical_function operator+(const TermType &term) {
        logical_function ret(*this);
        ret += term;
        return ret;
    }
    logical_function  operator+(const logical_function<TermType> &func) {
        logical_function ret(*this);
        ret += func;
        return ret;
    }
    logical_function& operator+=(const TermType &term) { add(term); return *this; }
    logical_function& operator+=(const logical_function &func) { add(func); return *this; }
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
