class fetch_queue {
	private:
		unsigned int *q;	// fetch queue
		unsigned int size;	// size of fetch queue
		unsigned int head;	// head pointer
		unsigned int tail;	// tail pointer
		int length;		// number of instructions in fetch queue

	public:
		fetch_queue(unsigned int size);		// constructor

		bool enough_space(unsigned int i);	// returns true if the fetch queue has enough space for i instructions
		void push(unsigned int index);		// push an instruction (its payload buffer index) into the fetch queue

		bool bundle_ready(unsigned int i);	// returns true if the fetch queue has enough instructions to form a bundle of i instructions
		unsigned int pop();			// pop an instruction (its payload buffer index) from the fetch queue

		void flush();				// flush the fetch queue (make it empty)
};
