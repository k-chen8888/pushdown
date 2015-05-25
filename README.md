# PushDown
A test pushdown automata library in C++

### Usage

######[1] Building an automata
Any type:
- Vector of elements
- Vector of "pairs"
  - Pairs are delimiters such that one of them "opens" a block and another "closes" a block
    - Examples: { }, ' ',
  - The first element of the vector is the delimiter character
- Comparator function
- Copy function
- toString function
- Destructor function

string:
- string object to parse
- Vector of "pairs"

######[2] Iterating through the automata
Whenever readNext() is called, the automata will go forward by 1 position in the string/vector
- If readNext() comes across a delimiter, it will greedily output any token it can find

The automata will not continue if it detects a syntax error
- Error code will be set depending on what type of syntax error was found