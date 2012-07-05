#ifndef QUINE_MCCLUSKEY_HPP
#define QUINE_MCCLUSKEY_HPP


#include <iostream>
#include <set>
#include <algorithm>
#include <stdexcept>
#include "logical_expr.hpp"


namespace quine_mccluskey {

using namespace std;
using namespace logical_expr;

class minimizer {
public:
    typedef logical_term<term_mark> term_type;
    typedef vector<term_type> set_type;
    typedef vector<set_type> table_type;

    minimizer() : min_level_(0) 
        { add_table(table_type()); make_min_table(); }
    minimizer(const logical_function<term_type> &function) : min_level_(0), func_(function)
        { add_table(table_type()); make_min_table(); }
    ~minimizer() {}

    int get_current_level() const { return min_level_; }
    void clear_table() {
        for( int i = 0; i < table_.size(); ++i )
            table_[i].clear();
    }

    template<typename T>
    static void make_unique(vector<T> &vec) {
        for( auto it = vec.begin(); it != vec.end(); ++it ) {
            auto rm_it = remove(it + 1, vec.end(), *it);
            vec.erase(rm_it, vec.end());
        }
    }

    // Return standard sum of products form
    const logical_function<term_type>& get_std_spf() { return get_std_spf(func_); }
    const logical_function<term_type>& get_std_spf(const logical_function<term_type> &func) {
        stdspf_.clear();
        for( int i = 0; i < std::pow(2, func.term_size()); ++i ) {
            logical_term<term_mark>::arg_type arg(func.term_size(), i);
            if( func(arg) )
                stdspf_ += logical_term<term_mark>(arg);
        }
        return stdspf_;
    }

    const table_type& make_min_table() {
        table_[0].resize(func_.term_size() + 1, set_type());
        for( auto term : stdspf_ )
            table_[0][term.num_of_value(true)].push_back(term);
        return table_[0];
    }
    
    // Try to find prime implicants
    // Return true while trying to find them
    // Return false if it finished
    bool find_prime_implicants() {
        table_type next_table;
        next_table.resize(func_.term_size(), set_type());
        int count = 0;
        for( int i = 0; i+1 < table_[min_level_].size(); ++i ) {
            for( int j = 0; j < table_[min_level_][i].size(); ++j ) {
                for( int k = 0; k < table_[min_level_][i+1].size(); ++k ) {
                    try {
                        // Throw an exception if could not minimize
                        auto term = onebit_minimize(table_[min_level_][i][j], table_[min_level_][i+1][k], false);
                        cout << "MINIMIZE(" << table_[min_level_][i][j] << ", " << table_[min_level_][i+1][k] << ") = " << term << endl;
                        next_table[term.num_of_value(true)].push_back(term);
                        ++count;
                        // Mark the used term for minimization
                        property_set(table_[min_level_][i][j], true);
                        property_set(table_[min_level_][i+1][k], true);
                    } catch( std::exception &e ) {}
                }
            }
        }
        if( count ) {
            ++min_level_;
            add_table(next_table);
        }
        return (count ? true : false);
    }

    void get_prime_implicants() {
        for( ; find_prime_implicants(); );
        for( auto table : table_  ) {
            for( auto set : table ) {
                for( const logical_term<term_mark> &term : set ) {
                    if( !property_get(term) )
                        prime_imp.push_back(term);
                }
            }
        }
        make_unique(prime_imp);
    }

    void print_prime_implicants() const {
        for( auto term : prime_imp ) cout << term << " ";
        cout << endl;
    }
    
    void print_all_table() const {
        for( auto table : table_ ) {
            for( auto set : table ) {
                for( auto term : set ) 
                    cout << term << endl;
            }
        }
    }
    
private:
    void add_table(const table_type& table) { table_.push_back(table); }
    int min_level_;
    logical_function<term_type> func_, stdspf_, minimized;
    vector<table_type> table_;
    set_type prime_imp;
};



}   // namespace quine_mccluskey


#endif  // QUINE_MCCLUSKEY_HPP
