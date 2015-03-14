#ifndef RfidReader_h
#define RfidReader_h

class RfidReader
{
	public:
		RfidReader();
		void resetRfid();
		bool scan();
		char tag[10];
	private:
		bool compareCurrentKey();
		void storeCurrentKey();
		int bytesread;
		int val;
		int dontSpam;
};

#endif
