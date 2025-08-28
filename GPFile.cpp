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

struct measure {
	
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
		
		GPFile() {
			
		}
};