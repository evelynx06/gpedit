#ifndef GP_FILE_H
#define GP_FILE_H

#include <vector>
#include <fstream>

enum MeasureHeaderFlags {
	gp_measure_keysig_numerator = 0x01,
	gp_measure_keysig_denominator = 0x02,
	gp_measure_repeat_begin = 0x04,
	gp_measure_repeat_end = 0x08,
	gp_measure_altend_number = 0x10,
	gp_measure_marker = 0x20,
	gp_measure_tonality = 0x40,
	gp_measure_double_bar = 0x80
};
enum TrackHeaderFlags {
	gp_track_drums = 0x01,
	gp_track_12_string = 0x02,
	gp_track_banjo = 0x04,
};
enum BeatFlags {
	gp_beat_is_dotted = 0x01,
	gp_beat_has_chord = 0x02,
	gp_beat_has_text = 0x04,
	gp_beat_has_effects = 0x08,
	gp_beat_has_mix_change = 0x10,
	gp_beat_is_tuplet = 0x20,
	gp_beat_is_empty_or_rest = 0x40,
};
enum BeatEffectFlags {
	gp_beatfx_vibrato = 0x01,
	gp_beatfx_wide_vibrato = 0x02,
	gp_beatfx_natural_harmonic = 0x04,
	gp_beatfx_artificial_harmonic = 0x08,
	gp_beatfx_fade_in = 0x10,
	gp_beatfx_tremolo_or_tap = 0x20,
	gp_beatfx_strum = 0x40,
};
enum NoteFlags {
	gp_note_has_independent_duration = 0x01,
	gp_note_is_heavy_accent = 0x02,
	gp_note_is_ghost = 0x04,
	gp_note_has_effects = 0x08,
	gp_note_has_dynamics = 0x10,
	gp_note_has_fret = 0x20,
	gp_note_is_accent = 0x40,
	gp_note_has_fingering = 0x80
};
enum NoteEffectFlags {
	gp_notefx_bend = 0x01,
	gp_notefx_hammer_pull = 0x02,
	gp_notefx_slide = 0x04,
	gp_notefx_let_ring = 0x08,
	gp_notefx_grace_note = 0x10
};

enum NoteDuration {
	gp_duration_whole = -2,
	gp_duration_half = -1,
	gp_duration_quarter = 0,
	gp_duration_eighth = 1,
	gp_duration_sixteenth = 2,
	gp_duration_thirty_second = 3,
	gp_duration_sixty_fourth = 4
};
enum StrumSpeed {
	gp_strum_none = 0,
	gp_strum_hundred_twenty_eighth = 1,
	gp_strum_sixty_fourth = 2,
	gp_strum_thirty_second = 3,
	gp_strum_sixteenth = 4,
	gp_strum_eighth = 5,
	gp_strum_quarter = 6
};
enum NoteType {
	gp_notetype_normal = 1,
	gp_notetype_tied = 2,
	gp_notetype_dead = 3
};
enum BendType {
	gp_bendtype_none = 0,
	gp_bendtype_bend = 1,
	gp_bendtype_bend_release = 2,
	gp_bendtype_bend_release_bend = 3,
	gp_bendtype_prebend = 4,
	gp_bendtype_prebend_release = 5,
	// following are for tremolo bar only, and actually not part of the gp3 spec
	gp_bendtype_dip = 6,
	gp_bendtype_dive = 7,
	gp_bendtype_release_up = 8,
	gp_bendtype_inverted_dip = 9,
	gp_bendtype_return = 10,
	gp_bendtype_release_down = 11
};

struct MidiChannel {
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

struct MeasureHeader {
	unsigned char measureFlags;	// indicates what data is present in the measure header
									// gp_measure_repeat_begin and GP_MEASURE_DOUBLE_BAR don't have any associated data
	unsigned char keysigNumerator;
	unsigned char keysigDenominator;
	unsigned char repeatEnd;	// number of repeats
	unsigned char altendNumber;
	std::string markerName;
	unsigned char markerColor[4];	// red, green, blue, white (white is always 0)
	unsigned char tonalityRoot;	// key change: key signature root
	unsigned char tonalityType;	// key change: key signature type
};

struct TrackHeader {
	unsigned char trackFlags;	// indicates if the track is one of the special types:
									// drums, 12 string guitar or banjo
									
	std::string name;
	int stringCount;
	int stringTuning[7];	// stored from thinnest to thickest
	int midiPort;
	
	int midiChannel;
	int midiEffectsChannel;
	int fretCount;
	int capo;
	unsigned char color[4];	// red, green, blue, white (white is always 0)
};

struct Chord {
	bool newFormat;		// !! must be 0 for GP3 format !!
	std::string name;
	int diagramFirstFret;	// the fret to start diagram at,
									// if this is 0 there is no diagram, and frets are not read
	int diagramFrets[6];	// the frets played on each string, -1 means not played
};

struct BeatEffects {
	unsigned char beatEffectFlags;
	unsigned char tremoloOrTap;	// only if gp_beatfx_tremolo_or_tap
										// 0 = tremolo, 1 = tap, 2 = slap, 3 = pop
	int tremoloValue;	// only if tremoloOrTap == 0
						// 50 = semitone, 100 = whole tone, 150 = 3 semitones, 200 = 2 whole tones, ...
	enum StrumSpeed strumDown;	// only if gp_beatfx_strum
	enum StrumSpeed strumUp;	// only if gp_beatfx_strum
};

struct MixChange {
	// the new values, -1 if unchanged
	char instrument;
	char volume;
	char balance;
	char chorus;
	char reverb;
	char phaser;
	char tremolo;
	int tempo;
	
	// duration of change, as number beats
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

struct BendPoint {
	int position;
	int value;
	bool vibrato;
};

struct Bend {
	enum BendType type;
	int value;
	int pointCount;
	std::vector<BendPoint> points;
};

struct GraceNote {
	char fret;
	unsigned char dynamic;
	unsigned char transition;
	unsigned char duration;
};

struct Note {
	unsigned char noteFlags;
	
	enum NoteType noteType;	// only if gp_note_has_fret
	
	// only if gp_note_has_independent_duration
	enum NoteDuration duration;
	char tupletDivision;
	
	char dynamic; // only if gp_note_has_dynamics
	char fretNumber;	// only if gp_note_has_fret
	
	// only if gp_note_has_fingering
	char leftHandFinger;
	char rightHandFinger;
	
	
	// only if gp_note_has_effects
	unsigned char noteEffectFlags;
	Bend noteBend;	// only if gp_notefx_bend
	GraceNote grace;	// only if gp_notefx_grace_note
};

struct Notes {
	// indicates which strings have associated notes
	// 0x40 is the thinnest string (index 0)
	// 0x20 is the next thinnest (index 1)
	// and so on...
	unsigned char stringsPlayed;
	
	Note strings[7];	// stored from thinnest to thickest
};

struct Beat {
	unsigned char beatFlags;		// indicates what data is present in the beat
									// gp_beat_is_dotted doesn't have any associated data
	bool isRest;	// only if gp_beat_is_empty_or_rest
	enum NoteDuration duration;
	int tupletDivision;	// only if gp_beat_is_tuplet
	Chord chordDiagram;	// only if gp_beat_has_chord
	std::string text;	// only if gp_beat_has_text
	
	// only if gp_beat_has_effects
	BeatEffects effects;

	MixChange mixTableChange;	// only if gp_beat_has_mix_change
	Notes beatNotes;
};

struct Measure {
	int beatCount;
	std::vector<Beat> beats;
};



class GPFile {
	public:
		std::string version;
		struct {
			std::string title;
			std::string subtitle;
			std::string artist;
			std::string album;
			std::string words;
			std::string copyright;
			std::string tabbedBy;
			std::string instructions;
			std::vector<std::string> notice;
		} metadata;
		
		bool tripletFeel;
		int tempo;
		int key;	// key signature represented as the number of sharps or flats (negative numbers for flats)
		
		MidiChannel midiChannels[4][16];
		
		int measureCount;
		int trackCount;
		
		std::vector<MeasureHeader> measureHeaders;
		std::vector<TrackHeader> trackHeaders;
		
		std::vector<std::vector<Measure>> measures;	// measures[measureCount][trackCount]
		
		GPFile() { }
		GPFile(std::ifstream &fileStream);
		
		int read_song(std::ifstream &fileStream);
		int read_version(std::ifstream &fileStream);
		int read_metadata(std::ifstream &fileStream);
		int read_midi_channels(std::ifstream &fileStream);
		MeasureHeader read_measure_header(std::ifstream &fileStream);
		TrackHeader read_track_header(std::ifstream &fileStream);
		Measure read_measure(std::ifstream &fileStream);
		Beat read_beat(std::ifstream &fileStream);
		Chord read_chord(std::ifstream &fileStream);
		BeatEffects read_beat_effects(std::ifstream &fileStream);
		MixChange read_mix_change(std::ifstream &fileStream);
		Notes read_notes(std::ifstream &fileStream);
		Note read_note(std::ifstream &fileStream);
		Bend read_bend(std::ifstream &fileStream);
		GraceNote read_grace_note(std::ifstream &fileStream);
};

#endif // !GP_FILE_H