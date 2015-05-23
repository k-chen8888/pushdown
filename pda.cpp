/* Imports */

#include <iostream>
#include <string>
#include <vector>

// Class header
#include "pda.h"

// Namespaces
using namespace std;


/************************************************
 * Standard template T
 * General-purpose PDA
 ************************************************/

// Constructor
template <typename T>
PDA<T>::PDA(vector<T> src, vector<T> p, comparatorF co, copyF cp, toStringF ts, destructorF de)
{
	// Load info
	this->source = src;
	this->pairs = p;
	
	// Stack... is already initialized to an empty vector
	
	// Tracking
	this->start = 0;
	this->pos = 0;
	this->esc = false;
	
	// Comparator, copy, toString, and destructor functions
	this->comp = co;
	this->cpy = cp;
	this->tstr = ts;
	this->destr = de;
	
	// Error codes
	this->err = 0;
	
	// Delimiter index
	this->idelim = 0;
};

/*******************************************
 * Functions
 *******************************************/

/* Traverse automata */

// Read next element from source
template <typename T>
vector<T> PDA<T>::readNext()
{
	vector<T> out;
	
	// Normal operation
	if(this->esc) // Check for escaped character
	{
		this->esc = false;
		
		// Nothing else to do
		this->pos += 1;
		return out;
	}
	else
	{
		// Check for escape character
		if( this->comp( &(this->source[this->pos]), &(this->pairs[0]) ) == 0 )
		{
			this->esc = true;
			
			// Nothing else to do
			this->pos += 1;
			return out;
		}
		else
		{
			// Check for opening delimiter
			for(unsigned int i = 1; i < this->pairs.size(); i += 2)
			{
				// Check for opening delimiters that pair with themselves
				if( this->stack.size() > 0 )
				{
					if( this->comp( &(this->source[this->pos]), &(this->pairs[i + 1]) ) == 0 )
					{
						this->pop();
						
						// Build token
						out = this->getPortion(true);
						
						// Clean up and return
						this->pos += 1;
						return out;
					}
				}
				
				// Normal opening delimiter
				if( this->comp( &(this->source[this->pos]), &(this->pairs[i]) ) == 0 )
				{
					this->push(i);
					
					// Try to build a token
					out = this->getPortion(true);
					
					// Clean up and return
					this->pos += 1;
					return out;
				}
			}
			
			// Check for closing delimiter
			for(unsigned int i = 2; i < this->pairs.size(); i += 2)
			{
				if( this->comp( &(this->source[this->pos]), &(this->pairs[i]) ) == 0 )
				{
					if( this->stack.size() > 0 )
					{
						unsigned int op_index = this->stack.back();
						if(op_index == i - 1)
						{
							this->pop();
						}
						else // Invalid closing delimiter
						{
							// Report error, stop traversal, and return
							this->err = this->mismatchErr(this->pairs[op_index], this->pairs[i]);
							return out;
						}
						
						// Build token
						out = this->getPortion(true);
						
						// Clean up and return
						this->pos += 1;
						return out;
					}
					else
					{
						// Report error, stop traversal, and return
						this->err = this->noStartErr(this->pairs[i]);
						return out;
					}
				}
			}
			
			// Found nothing of the sort
			this->pos += 1;
		}
	}
	
	// End of source?
	if(this->pos >= this->source.size())
	{
		// Check for errors at the end of traversal
		if(this->stack.size() > 0)
		{
			this->err = this->noCloseErr();
			return out;
		}
		
		// Try to output a token
		out = this->getPortion(true);
		return out;
	}
	
	// Final return
	return out;
};

// Add index of a delimiter to the stack
// Records what was pushed
template <typename T>
void PDA<T>::push(int index)
{
	this->stack.push_back(index);
	this->idelim = index;
};

// Remove index of a delimiter from the stack when its complement is found
// Resets the saved delimiter index to 0
template <typename T>
void PDA<T>::pop()
{
	this->stack.pop_back();
	this->idelim = 0;
};

/* Reporting */

// Get current position of automata
template <typename T>
unsigned int PDA<T>::getPos()
{
	return this->pos;
};

// Get error code
template <typename T>
int PDA<T>::getErr()
{
	return this->err;
};

// Get the index of the last delimiter to be popped off of the stack
template <typename T>
unsigned int PDA<T>::lastDelim()
{
	return this->idelim;
};

// Get a portion of source from this->start to this->pos as a vector (non-empty if this->start > this->pos)
// Update start if update == true
template <typename T>
vector<T> PDA<T>::getPortion(bool update)
{
	vector<T> out;
	
	for(unsigned int i = this->start; i < this->pos; i++)
	{
		// Add to output vector
		T* temp;
		temp = (T*)( this->cpy( (void*)&(this->source[i]) ) );
		out.push_back(*temp);
	}
	
	// Update start if necessary
	if(update)
	{
		this->start = this->pos + 1;
	}
	
	return out;
};

// Report starting delimiter missing
template <typename T>
int PDA<T>::noStartErr(T close)
{
	cout << "[Error] Non-escaped delimiter " << this->tstr((void*)(&close)) << " has no starting complement\n";
	
	return -1;
};

// Report closing delimiter missing
template <typename T>
int PDA<T>::noCloseErr()
{
	cout << "[Error] Non-escaped delimiter(s) [ ";
	for(unsigned int i = 0; i < this->stack.size(); i++)
	{
		int j = this->stack[i];
		cout << this->tstr( (void*)&(this->pairs[j]) ) << " ";
	}
	cout << "] do(es) not have a closing complement\n";
	
	return -2;
};

// Report starting/closing delimiter pair mismatch
template <typename T>
int PDA<T>::mismatchErr(T start, T close)
{
	cout << "[Error] Starting delimiter " << this->tstr((void*)(&start)) << " does not pair with closing delimiter " << this->tstr((void*)(&close)) << "\n";
	
	return -3;
};

/* Destructor */
template <typename T>
PDA<T>::~PDA()
{
	// Nothing to do, really
};


/************************************************
 * Specialized type string
 * Source is a string, delimiters are characters
 ************************************************/

// Constructor
inline
PDA<string>::PDA(string src, vector<char> p)
{
	// Load info
	this->source = src;
	this->pairs = p;
	
	// Stack... is already initialized to an empty vector
	
	// Tracking
	this->start = 0;
	this->pos = 0;
	this->esc = false;
	
	// No user-inputted comparator, copy, toString, and destructor functions needed
	
	// Error codes
	this->err = 0;
};

/*******************************************
 * Functions
 *******************************************/

/* Traverse automata */

// Read next element from source
inline
string PDA<string>::readNext()
{
	string out;
	
	// Normal operation
	if(this->esc) // Check for escaped character
	{
		this->esc = false;
		
		// Nothing else to do
		this->pos += 1;
		return out;
	}
	else
	{
		// Check for escape character
		if( this->source.at(this->pos) == this->pairs[0] )
		{
			this->esc = true;
			
			// Nothing else to do
			this->pos += 1;
			return out;
		}
		else
		{
			// Check for opening delimiter
			for(unsigned int i = 1; i < this->pairs.size(); i += 2)
			{
				// Check for opening delimiters that pair with themselves
				if( this->stack.size() > 0 )
				{
					if( this->source.at(this->pos) == this->pairs[i + 1] )
					{
						this->pop();
						
						// Build token
						out = this->getPortion(true);
						
						// Clean up and return
						this->pos += 1;
						return out;
					}
				}
				
				// Normal opening delimiter
				if( this->source.at(this->pos) == this->pairs[i] )
				{
					this->push(i);
					
					// Try to build a token
					out = this->getPortion(true);
					
					// Clean up and return
					this->pos += 1;
					return out;
				}
			}
			
			// Check for closing delimiter
			for(unsigned int i = 2; i < this->pairs.size(); i += 2)
			{
				if( this->source.at(this->pos) == this->pairs[i] )
				{
					if( this->stack.size() > 0 )
					{
						unsigned int op_index = this->stack.back();
						if(op_index == i - 1) // Pop if correct delimiter pairing
						{
							this->pop();
						}
						else // Invalid closing delimiter
						{
							// Report error, stop traversal, and return
							this->err = this->mismatchErr(this->pairs[op_index], this->pairs[i]);
							return out;
						}
						
						// Build token
						out = this->getPortion(true);
						
						// Clean up and return
						this->pos += 1;
						return out;
					}
					else
					{
						// Report error, stop traversal, and return
						this->err = this->noStartErr(this->pairs[i]);
						return out;
					}
				}
			}
			
			// Found nothing of the sort
			this->pos += 1;
		}
	}
		
	// End of source?
	if(this->pos >= this->source.size())
	{
		// Check for errors at the end of traversal
		if(this->stack.size() > 0)
		{
			this->err = this->noCloseErr();
			return out;
		}
		
		// Try to output a token
		out = this->getPortion(true);
		return out;
	}
	
	// Final return
	return out;
};

// Add index of a delimiter to the stack
// Records what was pushed
inline
void PDA<string>::push(int index)
{
	this->stack.push_back(index);
	this->idelim = index;
};

// Remove index of a delimiter from the stack when its complement is found
// Resets the saved delimiter index to 0
inline
void PDA<string>::pop()
{
	this->stack.pop_back();
	this->idelim = 0;
};

/* Reporting */

// Get current position of automata
inline
unsigned int PDA<string>::getPos()
{
	return this->pos;
};

// Get error code
inline
int PDA<string>::getErr()
{
	return this->err;
};

// Get the index of the last delimiter to be popped off of the stack
inline
unsigned int PDA<string>::lastDelim()
{
	return this->idelim;
};

// Get a portion of source from this->start to this->pos as a vector (non-empty if this->start > this->pos)
// Update start if update == true
inline
string PDA<string>::getPortion(bool update)
{
	string out = this->source.substr(this->start, this->pos - this->start);
	
	// Update start if necessary
	if(update)
	{
		this->start = this->pos + 1;
	}
	
	return out;
};

// Report starting delimiter missing
inline
int PDA<string>::noStartErr(char close)
{
	cout << "[Error] Non-escaped delimiter " << close << " has no starting complement\n";
	
	return -1;
};

// Report closing delimiter missing
inline
int PDA<string>::noCloseErr()
{
	cout << "[Error] Non-escaped delimiter(s) [ ";
	for(unsigned int i = 0; i < this->stack.size(); i++)
	{
		int j = this->stack[i];
		cout << this->pairs[j] << " ";
	}
	cout << "] do(es) not have a closing complement\n";
	
	return -2;
};

// Report starting/closing delimiter pair mismatch
inline
int PDA<string>::mismatchErr(char start, char close)
{
	cout << "[Error] Starting delimiter " << start << " does not pair with closing delimiter " << close << "\n";
	
	return -3;
};

/* Destructor */
inline
PDA<string>::~PDA()
{
	// Nothing to do, really
};