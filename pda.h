#ifndef PDA_H
#define PDA_H


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
		
		// Error checking
		// < 0 means error, do not continue
		int err;
		
		// Index of last opening delimiter popped
		unsigned int odelim;
		
		// Private default constructor
		PDA() { }
		
	public:
		/* Constructor */
		PDA(std::vector<T> src, std::vector<T> p, comparatorF co, copyF cp, toStringF ts, destructorF de)
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
			this->odelim = 0;
		};
		
		/*******************************************
		 * Functions
		 *******************************************/
		
		/* Traverse automata */
		
		// Read next element from source
		std::vector<T> readNext()
		{
			std::vector<T> out;
			
			// Do not proceed if error code is set or end of source is reached
			if(this->err < 0 || this->pos > this->source.size())
				return out;
			
			// Reset the last opening delimiter popped, assuming that the user has already accessed it
			if(this->odelim != 0)
				this->odelim = 0;
			
			// There's nothing to do if this element is escaped
			if(this->esc == true)
			{
				this->esc = false;
				
				// Clean up and end
				this->pos += 1;
				if(this->pos >= this->source.size() && this->stack.size() > 0)
				{
					// Unclosed delimiter error
					this->err = this->noCloseErr();
				}
				return out;
			}
			
			// There's nothing to do if this is an escape element
			if( this->comp( &(this->source[this->pos]), &(this->pairs[0]) ) == 0 )
			{
				this->esc = true;
				
				// Clean up and end
				this->pos += 1;
				if(this->pos >= this->source.size() && this->stack.size() > 0)
				{
					// Unclosed delimiter error
					this->err = this->noCloseErr();
				}
				return out;
			}
			
			// Check for delimiters
			for(int i = 1; i < this->pairs.size(); i++)
			{
				// Is this a delimiter?
				if( this->comp( &(this->source[this->pos]), &(this->pairs[i]) ) == 0 )
				{
					// What kind of delimiter is this?
					if(i % 2 == 1) // Opening delimiter
					{
						// Check to see if it matches a closing delimiter
						if( this->comp( &(this->source[this->pos]), &(this->pairs[i + 1]) ) == 0 && this->stack.back() == i)
						{
							// Handle it like a closing delimiter if it is already on the stack AND it is at the top of the stack
							this->pop();
						}
						else
						{
							// Otherwise, handle it like an opening delimiter
							this->push(i);
						}
					}
					else           // Closing delimiter
					{
						// Pop only if valid closing delimiter
						if( this->stack.size() > 0 )
						{
							if(this->stack.back() == i - 1)
							{
								// Safe to pop()
								this->pop();
							}
							else
							{
								// This closing delimiter does not match the one found on top of the stack
								this->err = this->mismatchErr(this->pairs[this->stack.back()], this->pairs[i]);
								return out;
							}
						}
						else
						{
							// No opening delimiters found on the stack
							this->err = this->noStartErr(this->pairs[i]);
							return out;
						}
						
						// Save last index to be popped
						this->odelim = i - 1;
					}
					
					// Attempt to generate a token
					out = this->getPortion(true);
					
					// Clean up and end
					this->pos += 1;
					if(this->pos >= this->source.size() && this->stack.size() > 0)
					{
						// Unclosed delimiter error
						this->err = this->noCloseErr();
					}
					return out;
				}
			}
			
			// Clean up and end
			this->pos += 1;
			if(this->pos >= this->source.size() && this->stack.size() > 0)
			{
				// Unclosed delimiter error
				this->err = this->noCloseErr();
			}
			return out;
		};
		
		// Add index of a delimiter to the stack
		void push(unsigned int index)
		{
			this->stack.push_back(index);
			
			std::cout << "after push at " << this->pos << " [";
			for(int i = 0; i < this->stack.size(); i++)
			{
				std::cout << this->stack[i] << ", ";
			}
			std::cout << "]\n";
		};
		
		// Remove index of a delimiter from the stack when its complement is found
		// Resets the saved delimiter index to the last element of the stack
		void pop()
		{
			if(this->stack.size() > 0)
			{
				this->odelim = this->stack.back();
				this->stack.pop_back();
			}
			else // Cannot pop from empty stack
			{
				std::cout << "Nothing to pop from stack";
			}
			
			std::cout << "after pop at " << this->pos << " [";
			for(int i = 0; i < this->stack.size(); i++)
			{
				std::cout << this->stack[i] << ", ";
			}
			std::cout << "]\n";
		};
		
		/* Reporting */
		
		// Get current position of automata
		unsigned int getPos()
		{
			return this->pos;
		};
		
		// Get error code
		int getErr()
		{
			return this->err;
		};
		
		// Get the index of the last delimiter to be pushed onto the stack
		// 0 if the stack is empty
		unsigned int lastDelim()
		{
			if(this->stack.size() > 0)
				return this->stack.back();
			else
				return 0;
		};
		
		// Get the index of the last delimiter to be removed from the stack
		unsigned int lastRemoved()
		{
			return this->odelim;
		};
		
		// Get the depth of the stack
		unsigned int stackDepth()
		{
			return this->stack.size();
		};
		
		// Check if escape character flag is set
		bool isEsc()
		{
			return this->esc;
		};
		
		// Get a portion of source from this->start to this->pos as a vector (non-empty if this->start > this->pos)
		// Update start if update == true
		std::vector<T> getPortion(bool update)
		{
			std::vector<T> out;
			
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
		int noStartErr(T close)
		{
			std::cout << "[Error] Non-escaped delimiter " << this->tstr((void*)(&close)) << " has no starting complement\n";
			
			return -1;
		};
		
		// Report closing delimiter missing
		int noCloseErr()
		{
			std::cout << "[Error] Non-escaped delimiter(s) [ ";
			for(unsigned int i = 0; i < this->stack.size(); i++)
			{
				int j = this->stack[i];
				std::cout << this->tstr( (void*)&(this->pairs[j]) ) << " ";
			}
			std::cout << "] do(es) not have a closing complement\n";
			
			return -2;
		};
		
		// Report starting/closing delimiter pair mismatch
		int mismatchErr(T start, T close)
		{
			std::cout << "[Error] Starting delimiter " << this->tstr((void*)(&start)) << " does not pair with closing delimiter " << this->tstr((void*)(&close)) << "\n";
			
			return -3;
		};
		
		/* Destructor */
		~PDA()
		{
			// Nothing to do, really
		};
};


#endif