#ifndef PDA_H
#define PDA_H


/* Build a Pushdown Automata to parse an input
 * Written using a template so that anything can use it
 */


// Typedefs for comparator/destructor functions
typedef int (*comparatorF)(void*, void*);
typedef void* (*copyF)(void*);
typedef std::string (*toStringF)(void*);
typedef void (*destructorF)(void*);


/************************************************
 * Standard template T
 * General-purpose PDA
 ************************************************/
template <typename T>
class PDA
{
	private:
		std::vector<T> source;           // Source to read from (generally some kind of list or string)
		std::vector<unsigned int> stack; // Stack used to keep track of delimiter pairs, array of indices from delimiter pairs vector
		std::vector<T> pairs;            // Token pairs, store the escape delimiter in index 0
		int start;                       // Starting position of valid token
		unsigned int pos;                // Current read position of PDA
		bool esc;                        // True if an escape character was found
		
		// Comparator, copy, toString, and destructor functions
		comparatorF comp;                // "less than" returns < 0; "greater than" returns > 0; "equal to" returns 0
		copyF cpy;
		toStringF tstr;
		destructorF destr;
		
		// Error checking; < 0 means error, do not continue
		int err;
		
		// Private default constructor
		PDA() { }
		
	public:
		// Constructor
		PDA(std::vector<T> src, std::vector<T> p, comparatorF co, copyF cp, toStringF ts, destructorF de);
		
		/*******************************************
		 * Functions
		 *******************************************/
		
		/* Traverse automata */
		
		// Read next element from source
		std::vector<T> readNext();
		
		// Add index of a delimiter to the stack
		void push(int index);
		
		// Remove index of a delimiter from the stack when its complement is found
		void pop();
		
		/* Reporting */
		
		// Get current position of automata
		unsigned int getPos();
		
		// Get error code
		int getErr();
		
		// Get a portion of source from this->start to this->pos as a vector (non-empty if this->start > this->pos)
		// Update start if update == true
		std::vector<T> getPortion(bool update);
		
		// Report starting delimiter missing
		int noStartErr(T close);
		
		// Report closing delimiter missing
		int noCloseErr();
		
		// Report starting/closing delimiter pair mismatch
		int mismatchErr(T start, T close);
		
		/* Destructor */
		~PDA();
};


/************************************************
 * Specialized type string
 * Source is a string, delimiters are characters
 ************************************************/
template <>
class PDA<std::string>
{
	private:
		std::string source;               // Source to read from (generally some kind of list or string)
		std::vector<unsigned int> stack;  // Stack used to keep track of delimiter pairs, array of indices from delimiter pairs vector
		std::vector<char> pairs;          // Token pairs, store the escape delimiter in index 0
		int start;                        // Starting position of valid token
		unsigned int pos;                 // Current read position of PDA
		bool esc;                         // True if an escape character was found
		
		// Error checking; < 0 means error, do not continue
		int err;
		
		// Private default constructor
		PDA() { }
		
	public:
		// Constructor
		PDA(std::string, std::vector<char> p);
		
		/*******************************************
		 * Functions
		 *******************************************/
		
		/* Traverse automata */
		
		// Read next element from source
		std::string readNext();
		
		// Add index of a delimiter to the stack
		void push(int index);
		
		// Remove index of a delimiter from the stack when its complement is found
		void pop();
		
		/* Reporting */
		
		// Get current position of automata
		unsigned int getPos();
		
		// Get error code
		int getErr();
		
		// Get a portion of source from this->start to this->pos as a vector (non-empty if this->start > this->pos)
		// Update start if update == true
		std::string getPortion(bool update);
		
		// Report starting delimiter missing
		int noStartErr(char close);
		
		// Report closing delimiter missing
		int noCloseErr();
		
		// Report starting/closing delimiter pair mismatch
		int mismatchErr(char start, char close);
		
		/* Destructor */
		~PDA();
};


#endif