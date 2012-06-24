
#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/dynamic_bitset.hpp>

using namespace std;

namespace quine_mccluskey {

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
    function_parser(const string &expr) : expr_(expr) {}
    ~function_parser() {}

    void set_expression(const string &expr) { expr_ = expr; }
    string set_expression() { return expr_; }

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
        for( unsigned int i = 0; i < result.size(); ++i )
            cout << "result[" << i << "] = " << result[i] << endl;
        //                    func-name  decl-vars  decl-terms
        return vector<string>{result[1], result[2], result[6]};
    }

    static result_type tokenizer(const vector<string> &untokenized) {
        std::vector<string> vars, terms;
        typedef boost::char_separator<char> char_separator;
        boost::tokenizer<char_separator> 
            var_tokenizer(untokenized[1], char_separator(",")), term_tokenizer(untokenized[2], char_separator("+"));
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
    string expr_;
};

template<char inverter = '^'>
class boolean_term {
public:
    typedef boost::optional<bool> value_type;    
    typedef boost::dynamic_bitset<> arg_type;
    boolean_term() {}
    boolean_term(const string &expr, int bitsize) : term_(bitsize, dont_care) { parse(expr); }

    int size() const { return term_.size(); }

    bool calculate(const arg_type &arg) const {
        if( arg.size() != term_.size() )
            throw std::runtime_error("target two operands are not same size");
        bool ret = true;
        for( int i = 0; i < term_.size(); ++i ) {
            ret = ret && (term_[i] ? arg[i] == term_[i] : true);
        }
        return ret;
    }

    value_type& operator[](int index) { return term_[index]; }
    friend ostream& operator<<(ostream &os, const boolean_term &bf) {
        for( auto b : bf.term_ ) {
            if( b ) cout << *b;
            else    cout << 'x';
        }
        return os;
    }
private:
    void parse(const string &expr) {
        bool invert = false;
        for( char var : expr ) {
            if( var == inverter ) {
                invert = true;
                continue;
            }
            term_[var - 'A'] = !invert;
            invert = false;
        }
    }
    static const boost::optional<bool> dont_care;
    vector<value_type> term_;
};

template<>
const boost::optional<bool> boolean_term<>::dont_care = boost::none;



}   // namespace quine_mccluskey

void print_truth_table(const quine_mccluskey::boolean_term<> &logterm) {
    for( int i = 0; i < std::pow(2, logterm.size()); ++i ) {
        quine_mccluskey::boolean_term<>::arg_type arg(logterm.size(), i);
        cout << "f = " << logterm << " <- ";
        for( int n = 0; n < arg.size(); ++n )
            cout << arg[n];
        cout << " = " << logterm.calculate(arg) << endl;
    }
}

int main(int argc, char **argv)
{
    int exit_code = EXIT_SUCCESS;
    try {
        cout << "Quine-McCluskey" << endl
             << "[*] Enter the formula to solve" << endl
             << "Input: " << flush;
        string line;
        getline(cin, line);
        auto token = quine_mccluskey::function_parser<'^', true>(line).parse();
        cout << "Declared variables: ";
        copy(token.first.begin(), token.first.end(), ostream_iterator<char>(cout, " "));
        cout << endl << "Using terms: ";
        copy(token.second.begin(), token.second.end(), ostream_iterator<string>(cout, " "));
        cout << endl << "Number of variables = " << token.first.size() << endl;;
        for( string term : token.second ) {
            cout << term << ":" << endl;
            quine_mccluskey::boolean_term<> logterm(term, token.first.size());
            print_truth_table(logterm);
        }
    }
    catch( std::exception &e ) {
        cerr << "[-] Exception: " << e.what() << endl;
        exit_code = EXIT_FAILURE;
    }
    return exit_code;
}

