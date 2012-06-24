
#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace quine_mccluskey {

// expr_mode is not used now. 
// But this will be used to change the expression of functions
enum expr_mode { alphabet_expr };

/*
 * String to be parsed has to be in the following form
 * ${Function-Name}(Variables-divided-by-',' ...) = ${TERMS} + ...
 * White spaces will be ignored
 * Default character to invert a variable is '^'
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
            (boost::format("(.+)\\((((\\s*[A-Z],)*)([A-Z]))\\)=(((%1%?[A-Z])+\\+)*((%1%?[A-Z])+))$") 
                % ( escape ? string{'\\', inverter} : string{inverter})).str(),
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

/*

template<char inverter = '^'>
class boolean_function {
public:
    boolean_function() {}
    boolean_function(const string &expr, int bitsize) {
        
    }
private:
    void parse(const string &expr, bitsize) {
        bool invert = false;
        boost::dynamic_bitset<> 
        for( char  ) {
            if( var == inverter ) {
                invert = true;
                break;
            }

            invert = false;
        }
    }
    
    boost::dynamic_bitset<> term_;
};
*/

}   // namespace quine_mccluskey

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
        cout << endl;
    }
    catch( std::exception &e ) {
        cerr << "[-] Exception: " << e.what() << endl;
        exit_code = EXIT_FAILURE;
    }
    return exit_code;
}

