#include <iostream>

using namespace std;

struct midiChannel {
	int instrument;
	unsigned char volume;
	unsigned char balance;
	unsigned char chorus;
	unsigned char reverb;
	unsigned char phaser;
	unsigned char tremolo;
	unsigned char blank1;
	unsigned char blank2;
};

enum MeasureHeaderFlags {
	KEYSIG_NUMERATOR = 0x01,
	KEYSIG_DENOMINATOR = 0x02,
	REPEAT_BEGIN = 0x04,
	REPEAT_END = 0x08,
	ALTEND_NUMBER = 0x10,
	MARKER = 0x20,
	TONALITY = 0x40,
	DOUBLE_BAR = 0x80
};

struct measureHeader {
	unsigned char flags;		// indicates what data is present in the measure header
									// REPEAT_BEGIN and DOUBLE_BAR don't have any associated data
	unsigned char keysigNumerator;
	unsigned char keysigDenominator;
	unsigned char repeatEnd;	// number of repeats
	unsigned char altendNumber;
	string markerName;
	unsigned char markerColor[4];	// red, green, blue, white (white is always 0)
	unsigned char tonalityRoot;	// key change: key signature root
	unsigned char tonalityType;	// key change: key signature type
};

class GPFile {
	public:
		string version;
		struct {
			string title;
			string subtitle;
			string artist;
			string album;
			string words;
			string copyright;
			string tabbedBy;
			string instructions;
			string notice[];
		} metadata;
		
		bool tripletFeel;
		int tempo;
		int key;
		
		midiChannel midiChannels[4][16];
		
		int measureCount;
		int trackCount;
		
		measureHeader measureHeaders[];
		
		GPFile() {
			
		}
};