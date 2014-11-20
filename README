#
#
#   README
#

[*] Summary
    An implementation of Quine-McCluskey algorithm
    Simplification of logical expressions

[*] Build
    Just run make command
    $ make
    ./qm, the executable will be created
    if you need to specify a compiler
    $ make CXX="COMPILER YOU WANT TO USE"

[+] Requirements
    [+] Compile Time
        [*] Make
        [*] C++11 compiler
        [*] Boost C++ Libraries
            Almost all the libraries used by this are header-only.
            But you need to build boost_regex and boost_program_options
            Go to www.boost.org and get the source code to build it
    [+] Run time
        [*] Nothing. Just run it

[+] Tested environments
    [+] Compiler
        [*] gcc 4.6.3 / 4.7.1
        [*] clang version 3.1 / 3.2 (trunk 159072)
    [+] Operating System
        [*] Linux 3.4.6-1-ARCH i686 GNU/Linux (Arch Linux)
        [*] Linux 3.2.0-26-generic-pae i686 GNU/Linux (Ubuntu 12.04)
    [+] Libraries
        [*] Boost C++ Libraries 1.49.0 / 1.50.0 / 1.51.0

[+] Supported form(s) of expression
    [*] White spaces are ignored
    [+] Supported form is below:
        [*] In pseudo BNF
      -------------------------------------------------------------------------------------------
        <function>   ::= <func-name> '('<var-decl>')' '=' <expression>
        <func-name>  ::= [A-Za-z_-]+
        <var-decl>   ::= ([A-Z],)+ [A-Z]
        <expression> ::= <expression> + <expression> | <expression> + <term> | <term> + <term>
        <term>       ::= (\^?[A-Z])+
      -------------------------------------------------------------------------------------------

[*] Samples
    Input samples exist in sample/in[1-6].txt
    Also the expected output of each samples are in sample/out[1-6].txt

[*] License:
    The content of this repository is licensed under The BSD 2-Clause License

[*] TODO
    Modify the simplifier for more optimization
    Support Verilog source code
