#ifndef PDA_STRING_H
#define PDA_STRING_H


/************************************************
 * Specialized type string
 * Source is a string, delimiters are characters
 ************************************************/
template <>
class PDA<std::string>
{
	private:
		std::string source;              // Source to read from (generally some kind of list or string)
		std::vector<unsigned int> stack; // Stack used to keep track of delimiter pairs, array of indices from delimiter pairs vector
		std::vector<char> pairs;         // Token pairs, store the escape delimiter in index 0
		int start;                       // Starting position of valid token
		unsigned int pos;                // Current read position of PDA
		bool esc;                        // True if an escape character was found
		
		// Error checking
		// < 0 means error, do not continue
		int err;
		
		// Index of last opening delimiter popped
		unsigned int odelim;
		
		// Private default constructor
		PDA() { }
		
	public:
		/* Constructor */
		PDA(std::string src, std::vector<char> p)
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
			
			// Delimiter index
			this->odelim = 0;
		};
		
		/*******************************************
		 * Functions
		 *******************************************/
		
		/* Traverse automata */
		
		// Read next element from source
		std::string readNext()
		{
			std::string out;
			
			// Reset the last opening delimiter popped, assuming that the user has already accessed it
			if(this->odelim != 0)
				this->odelim = 0;
			
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
		void push(unsigned int index)
		{
			this->stack.push_back(index);
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
		std::string getPortion(bool update)
		{
			std::string out = this->source.substr(this->start, this->pos - this->start);
			
			// Update start if necessary
			if(update)
			{
				this->start = this->pos + 1;
			}
			
			return out;
		};
		
		// Report starting delimiter missing
		int noStartErr(char close)
		{
			std::cout << "[Error] Non-escaped delimiter " << close << " has no starting complement\n";
			
			return -1;
		};
		
		// Report closing delimiter missing
		int noCloseErr()
		{
			std::cout << "[Error] Non-escaped delimiter(s) [ ";
			for(unsigned int i = 0; i < this->stack.size(); i++)
			{
				int j = this->stack[i];
				std::cout << this->pairs[j] << " ";
			}
			std::cout << "] do(es) not have a closing complement\n";
			
			return -2;
		};
		
		// Report starting/closing delimiter pair mismatch
		int mismatchErr(char start, char close)
		{
			std::cout << "[Error] Starting delimiter " << start << " does not pair with closing delimiter " << close << "\n";
			
			return -3;
		};
		
		/* Destructor */
		~PDA()
		{
			// Nothing to do, really
		};
};


#endif