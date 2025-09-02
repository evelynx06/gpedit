#include <iostream>
#include <vector>

using namespace std;


enum MeasureHeaderFlags {
	GP_MEASURE_KEYSIG_NUMERATOR = 0x01,
	GP_MEASURE_KEYSIG_DENOMINATOR = 0x02,
	GP_MEASURE_REPEAT_BEGIN = 0x04,
	GP_MEASURE_REPEAT_END = 0x08,
	GP_MEASURE_ALTEND_NUMBER = 0x10,
	GP_MEASURE_MARKER = 0x20,
	GP_MEASURE_TONALITY = 0x40,
	GP_MEASURE_DOUBLE_BAR = 0x80
};
enum TrackHeaderFlags {
	GP_TRACK_DRUMS = 0x01,
	GP_TRACK_12_STRING = 0x02,
	GP_TRACK_BANJO = 0x04,
};
enum BeatFlags {
	GP_BEAT_IS_DOTTED = 0x01,
	GP_BEAT_HAS_CHORD = 0x02,
	GP_BEAT_HAS_TEXT = 0x04,
	GP_BEAT_HAS_EFFECTS = 0x08,
	GP_BEAT_HAS_MIX_CHANGE = 0x10,
	GP_BEAT_IS_TUPLET = 0x20,
	GP_BEAT_IS_EMPTY_OR_REST = 0x40,
};
enum BeatEffectFlags {
	GP_BEAT_FX_VIBRATO = 0x01,
	GP_BEAT_FX_WIDE_VIBRATO = 0x02,
	GP_BEAT_FX_NATURAL_HARMONIC = 0x04,
	GP_BEAT_FX_ARTIFICIAL_HARMONIC = 0x08,
	GP_BEAT_FX_FADE_IN = 0x10,
	GP_BEAT_FX_TREMOLO_OR_TAP = 0x20,
	GP_BEAT_FX_STRUM = 0x40,
};
enum NoteFlags {
	GP_NOTE_HAS_INDEPENDENT_DURATION = 0x01,
	GP_NOTE_IS_HEAVY_ACCENT = 0x02,
	GP_NOTE_IS_GHOST = 0x04,
	GP_NOTE_HAS_EFFECTS = 0x08,
	GP_NOTE_HAS_DYNAMICS = 0x10,
	GP_NOTE_HAS_FRET = 0x20,
	GP_NOTE_IS_ACCENT = 0x40,
	GP_NOTE_HAS_FINGERING = 0x80
};
enum NoteEffectFlags {
	GP_NOTE_FX_BEND = 0x01,
	GP_NOTE_FX_HAMMER_PULL = 0x02,
	GP_NOTE_FX_SLIDE = 0x04,
	GP_NOTE_FX_LET_RING = 0x08,
	GP_NOTE_FX_GRACE_NOTE = 0x10
};

enum Duration {
	GP_DURATION_WHOLE = -2,
	GP_DURATION_HALF = -1,
	GP_DURATION_QUARTER = 0,
	GP_DURATION_EIGHTH = 1,
	GP_DURATION_SIXTEENTH = 2,
	GP_DURATION_THIRTY_SECOND = 3,
	GP_DURATION_SIXTY_FOURTH = 4
};
enum StrumSpeed {
	GP_STRUM_NONE = 0,
	GP_STRUM_HUNDRED_TWENTY_EIGHTH = 1,
	GP_STRUM_SIXTY_FOURTH = 2,
	GP_STRUM_THIRTY_SECOND = 3,
	GP_STRUM_SIXTEENTH = 4,
	GP_STRUM_EIGHTH = 5,
	GP_STRUM_QUARTER = 6
};
enum NoteType {
	GP_NOTE_TYPE_NORMAL = 1,
	GP_NOTE_TYPE_TIED = 2,
	GP_NOTE_TYPE_DEAD = 3
};
enum BendType {
	GP_BEND_TYPE_NONE = 0,
	GP_BEND_TYPE_BEND = 1,
	GP_BEND_TYPE_BEND_RELEASE = 2,
	GP_BEND_TYPE_BEND_RELEASE_BEND = 3,
	GP_BEND_TYPE_PREBEND = 4,
	GP_BEND_TYPE_PREBEND_RELEASE = 5,
	GP_BEND_TYPE_DIP = 6,
	GP_BEND_TYPE_DIVE = 7,
	GP_BEND_TYPE_RELEASE_UP = 8,
	GP_BEND_TYPE_INVERTED_DIP = 9,
	GP_BEND_TYPE_RETURN = 10,
	GP_BEND_TYPE_RELEASE_DOWN = 11
};

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

struct measureHeader {
	unsigned char flags;		// indicates what data is present in the measure header
									// GP_MEASURE_REPEAT_BEGIN and GP_MEASURE_DOUBLE_BAR don't have any associated data
	unsigned char keysigNumerator;
	unsigned char keysigDenominator;
	unsigned char repeatEnd;	// number of repeats
	unsigned char altendNumber;
	string markerName;
	unsigned char markerColor[4];	// red, green, blue, white (white is always 0)
	unsigned char tonalityRoot;	// key change: key signature root
	unsigned char tonalityType;	// key change: key signature type
};

struct trackHeader {
	unsigned char flags;		// indicates if the track is one of the special types:
									// drums, 12 string guitar or banjo
	string name;
	int stringCount;
	int stringTuning[7];
	int midiPort;
	
	int midiChannel;
	int midiEffectsChannel;
	int fretCount;
	int capo;
	unsigned char color[4];	// red, green, blue, white (white is always 0)
};

struct chord {
	bool format;		// !! must be 0 for GP3 format !!
	string name;
	int diagramFirstFret;	// the fret to start diagram at,
									// if this is 0 there is no diagram, and frets are not read
	int diagramFrets[6];	// the frets played on each string, -1 means not played
};

struct mixChange {
	// the new values, -1 if unchanged
	char instrument;
	char volume;
	char balance;
	char chorus;
	char reverb;
	char phaser;
	char tremolo;
	int tempo;
	
	// duration of change, in beats
	// duration is only specified for values that changed
	char instrumentDuration;
	char volumeDuration;
	char balanceDuration;
	char chorusDuration;
	char reverbDuration;
	char phaserDuration;
	char tremoloDuration;
	char tempoDuration;
};

struct bendPoint {
	int position;
	int value;
	bool vibrato;
};

struct bend {
	enum BendType type;
	int value;
	int pointCount;
	vector<bendPoint> points;
};

struct graceNote {
	char fret;
	unsigned char dynamic;
	unsigned char transition;
	unsigned char duration;
};

struct note {
	unsigned char noteFlags;
	
	enum NoteType noteType;	// only if GP_NOTE_HAS_FRET
	
	// only if GP_NOTE_HAS_INDEPENDENT_DURATION
	char duration;
	char tupletDivision;
	
	char dynamic; // only if GP_NOTE_HAS_DYNAMICS
	char fretNumber;	// only if GP_NOTE_HAS_FRET
	
	// only if GP_NOTE_HAS_FINGERING
	char leftHandFinger;
	char rightHandFinger;
	
	
	// only if GP_NOTE_HAS_EFFECTS
	unsigned char noteEffectFlags;
	bend bend;	// only if GP_NOTE_FX_BEND
	graceNote graceNote;	// only if GP_NOTE_FX_GRACE_NOTE
};

struct notes {
	unsigned char stringsPlayed;	// indicates which strings have associated notes
	// the LSB represents string7,
	// the MSB doesn't represent any string
	
	note string7;
	note string6;
	note string5;
	note string4;
	note string3;
	note string2;
	note string1;
};

struct beat {
	unsigned char beatFlags;		// indicates what data is present in the beat
									// GP_BEAT_IS_DOTTED doesn't have any associated data
	bool isRest;	// only if GP_BEAT_IS_EMPTY_OR_REST
	enum Duration duration;
	int tupletDivision;	// only if GP_BEAT_IS_TUPLET
	chord chord;	// only if GP_BEAT_HAS_CHORD
	string text;	// only if GP_BEAT_HAS_TEXT
	
	// only if GP_BEAT_HAS_EFFECTS
	unsigned char beatEffectFlags;
	unsigned char tremoloOrTap;	// only if GP_BEAT_FX_TREMOLO_OR_TAP, 0=trem, 1=tap, 2=slap, 3=pop
	enum StrumSpeed strumUp;	// only if GP_BEAT_FX_STRUM
	enum StrumSpeed strumDown;	// only if GP_BEAT_FX_STRUM
	
	mixChange mixChange;	// only if GP_BEAT_HAS_MIX_CHANGE
	notes notes;
};

struct measure {
	int beatCount;
	vector<beat> beats;
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
			vector<string> notice;
		} metadata;
		
		bool tripletFeel;
		int tempo;
		int key;
		
		midiChannel midiChannels[4][16];
		
		int measureCount;
		int trackCount;
		
		vector<measureHeader> measureHeaders;
		vector<trackHeader> trackHeaders;
		
		vector<vector<measure>> measures;	// measures[measureCount][trackCount]
		
		GPFile() {
			
		}
};