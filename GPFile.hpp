#include <iostream>
#include <vector>
#include <fstream>

#include "gp_read.hpp"


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
		
		GPFile(std::ifstream &fileStream) { read_song(fileStream); }
		
		int read_song(std::ifstream &fileStream) {
			read_version(fileStream);
			read_metadata(fileStream);
			
			this->tripletFeel = gp_read::read_bool(fileStream);
			this->tempo = gp_read::read_int(fileStream);
			this->key = gp_read::read_int(fileStream);
			
			read_midi_channels(fileStream);
			
			this->measureCount = gp_read::read_int(fileStream);
			this->trackCount = gp_read::read_int(fileStream);
			
			for (int i = 0; i < this->measureCount; i++) {
				this->measureHeaders.push_back(read_measure_header(fileStream));
			}
			for (int i = 0; i < this->trackCount; i++) {
				this->trackHeaders.push_back(read_track_header(fileStream));
			}
			
			for (int i = 0; i < this->measureCount; i++) {	// loop through all measures
				std::vector<Measure> measureTracks;
				
				for (int j = 0; j < this->trackCount; j++) {	// for every measure, loop through all tracks
					measureTracks.push_back(read_measure(fileStream));
				}
				
				this->measures.push_back(measureTracks);
			}
			
			return 0;
		}
		
		
		int read_version(std::ifstream &fileStream) {
			this->version = gp_read::read_bytestring(fileStream);
			fileStream.seekg(30 - this->version.length(), std::ifstream::cur);
			
			if (this->version != "FICHIER GUITAR PRO v3.00") {
				std::cerr << "Incompatible file format '" << this->version << "'\n";
				return 1;
			}
			
			return 0;
		}
		
		int read_metadata(std::ifstream &fileStream) {
			this->metadata.title = gp_read::read_intbytestring(fileStream);
			this->metadata.subtitle = gp_read::read_intbytestring(fileStream);
			this->metadata.artist = gp_read::read_intbytestring(fileStream);
			this->metadata.album = gp_read::read_intbytestring(fileStream);
			this->metadata.words = gp_read::read_intbytestring(fileStream);
			this->metadata.copyright = gp_read::read_intbytestring(fileStream);
			this->metadata.tabbedBy = gp_read::read_intbytestring(fileStream);
			this->metadata.instructions = gp_read::read_intbytestring(fileStream);
			
			int noticeLength = gp_read::read_int(fileStream);
			for (int i = 1; i <= noticeLength; i++) {
				this->metadata.notice.push_back(gp_read::read_intbytestring(fileStream));
			}
			
			return 0;
		}
		
		int read_midi_channels(std::ifstream &fileStream) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 16; j++) {
					this->midiChannels[i][j] = {
						gp_read::read_int(fileStream),	// instrument
						gp_read::read_byte(fileStream),	// volume
						gp_read::read_byte(fileStream),	// balance
						gp_read::read_byte(fileStream),	// chorus
						gp_read::read_byte(fileStream),	// reverb
						gp_read::read_byte(fileStream),	// phaser
						gp_read::read_byte(fileStream),	// tremolo
						gp_read::read_byte(fileStream),	// blank1
						gp_read::read_byte(fileStream)	// blank2
					};
				}
			}
			
			return 0;
		}
		
		MeasureHeader read_measure_header(std::ifstream &fileStream) {
			MeasureHeader measure;
			measure.measureFlags = gp_read::read_byte(fileStream);
			
			if (measure.measureFlags & gp_measure_keysig_numerator) {
				measure.keysigNumerator = gp_read::read_byte(fileStream);
			}
			if (measure.measureFlags & gp_measure_keysig_denominator) {
				measure.keysigDenominator = gp_read::read_byte(fileStream);
			}
			if (measure.measureFlags & gp_measure_repeat_end) {
				measure.repeatEnd = gp_read::read_byte(fileStream);
			}
			if (measure.measureFlags & gp_measure_altend_number) {
				measure.altendNumber = gp_read::read_byte(fileStream);
			}
			if (measure.measureFlags & gp_measure_marker) {
				measure.markerName = gp_read::read_intbytestring(fileStream);
				measure.markerColor[0] = gp_read::read_byte(fileStream);	// red
				measure.markerColor[1] = gp_read::read_byte(fileStream);	// green
				measure.markerColor[2] = gp_read::read_byte(fileStream);	// blue
				measure.markerColor[3] = gp_read::read_byte(fileStream);	// white (always 0)
			}
			if (measure.measureFlags & gp_measure_tonality) {
				measure.tonalityRoot = gp_read::read_byte(fileStream);
				measure.tonalityType = gp_read::read_byte(fileStream);
			}
			
			return measure;
		}
		
		TrackHeader read_track_header(std::ifstream &fileStream) {
			TrackHeader track;
			track.trackFlags = gp_read::read_byte(fileStream);
			
			track.name = gp_read::read_bytestring(fileStream);
			fileStream.seekg(40 - track.name.length(), std::ifstream::cur);
			
			track.stringCount = gp_read::read_int(fileStream);
			for (int i = 0; i < 7; i++) {
				track.stringTuning[i] = gp_read::read_int(fileStream);
			}
			
			track.midiPort = gp_read::read_int(fileStream);
			track.midiChannel = gp_read::read_int(fileStream);
			track.midiEffectsChannel = gp_read::read_int(fileStream);
			
			track.fretCount = gp_read::read_int(fileStream);
			track.capo = gp_read::read_int(fileStream);
			
			track.color[0] = gp_read::read_byte(fileStream);	// red
			track.color[1] = gp_read::read_byte(fileStream);	// green
			track.color[2] = gp_read::read_byte(fileStream);	// blue
			track.color[3] = gp_read::read_byte(fileStream);	// white (always 0)
			
			return track;
		}
		
		Measure read_measure(std::ifstream &fileStream) {
			Measure measure;
			
			measure.beatCount = gp_read::read_int(fileStream);
			
			for (int i = 0; i < measure.beatCount; i++) {
				measure.beats.push_back(read_beat(fileStream));
			}
			
			return measure;
		}
		
		Beat read_beat(std::ifstream &fileStream) {
			Beat beat;
			
			beat.beatFlags = gp_read::read_byte(fileStream);
			
			if (beat.beatFlags & gp_beat_is_empty_or_rest) {
				beat.isRest = gp_read::read_bool(fileStream);
			}
			
			beat.duration = (NoteDuration)gp_read::read_signedbyte(fileStream);
			
			if (beat.beatFlags & gp_beat_is_tuplet) {
				beat.tupletDivision = gp_read::read_int(fileStream);
			}
			
			if (beat.beatFlags & gp_beat_has_chord) {
				beat.chordDiagram = read_chord(fileStream);
			}
			
			if (beat.beatFlags & gp_beat_has_text) {
				beat.text = gp_read::read_intbytestring(fileStream);
			}
			
			if (beat.beatFlags & gp_beat_has_effects) {
				beat.effects = read_beat_effects(fileStream);
			}
			
			if (beat.beatFlags & gp_beat_has_mix_change) {
				beat.mixTableChange = read_mix_change(fileStream);
			}
			
			beat.beatNotes = read_notes(fileStream);
			
			return beat;
		}
		
		Chord read_chord(std::ifstream &fileStream) {
			Chord chord;
			
			chord.newFormat = gp_read::read_bool(fileStream);
			if (chord.newFormat) {
				std::cerr << "Wrong chord diagram format!\n";
				return chord;
			}
			
			chord.name = gp_read::read_intbytestring(fileStream);
			chord.diagramFirstFret = gp_read::read_int(fileStream);
				
			if (chord.diagramFirstFret) {
				for (int i = 0; i < 6; i++) {
					chord.diagramFrets[i] = gp_read::read_int(fileStream);
				}
			}
			
			return chord;
		}
		
		BeatEffects read_beat_effects(std::ifstream &fileStream) {
			BeatEffects effects;
			
			effects.beatEffectFlags = gp_read::read_byte(fileStream);
			
			if (effects.beatEffectFlags & gp_beatfx_tremolo_or_tap) {
				effects.tremoloOrTap = gp_read::read_byte(fileStream);
				
				if (effects.tremoloOrTap == 0) {
					effects.tremoloValue = gp_read::read_int(fileStream);
				}
			}
			
			if (effects.beatEffectFlags & gp_beatfx_strum) {
				effects.strumDown = (StrumSpeed)gp_read::read_signedbyte(fileStream);
				effects.strumUp = (StrumSpeed)gp_read::read_signedbyte(fileStream);
			}

			return effects;
		}
		
		MixChange read_mix_change(std::ifstream &fileStream) {
			MixChange change;
			
			change.instrument = gp_read::read_signedbyte(fileStream);
			change.volume = gp_read::read_signedbyte(fileStream);
			change.balance = gp_read::read_signedbyte(fileStream);
			change.chorus = gp_read::read_signedbyte(fileStream);
			change.reverb = gp_read::read_signedbyte(fileStream);
			change.phaser = gp_read::read_signedbyte(fileStream);
			change.tremolo = gp_read::read_signedbyte(fileStream);
			change.tempo = gp_read::read_int(fileStream);
			
			if (change.instrument >= 0) {
				change.instrumentDuration = gp_read::read_signedbyte(fileStream);
			}
			if (change.volume >= 0) {
				change.volumeDuration = gp_read::read_signedbyte(fileStream);
			}
			if (change.balance >= 0) {
				change.balanceDuration = gp_read::read_signedbyte(fileStream);
			}
			if (change.chorus >= 0) {
				change.chorusDuration = gp_read::read_signedbyte(fileStream);
			}
			if (change.reverb >= 0) {
				change.reverbDuration = gp_read::read_signedbyte(fileStream);
			}
			if (change.phaser >= 0) {
				change.phaserDuration = gp_read::read_signedbyte(fileStream);
			}
			if (change.tremolo >= 0) {
				change.tremoloDuration = gp_read::read_signedbyte(fileStream);
			}
			if (change.tempo >= 0) {
				change.tempoDuration = gp_read::read_signedbyte(fileStream);
			}
			
			return change;
		}
		
		Notes read_notes(std::ifstream &fileStream) {
			Notes notes;
			
			notes.stringsPlayed = gp_read::read_byte(fileStream);
			
			for (int i = 0; i < 7; i++) {
				if (notes.stringsPlayed & (0x40 >> i)) {
					notes.strings[i] = read_note(fileStream);
				}
			}
			
			return notes;
		}
		
		Note read_note(std::ifstream &fileStream) {
			Note note;
			
			note.noteFlags = gp_read::read_byte(fileStream);
			
			if (note.noteFlags & gp_note_has_fret) {
				note.noteType = (NoteType)gp_read::read_byte(fileStream);
			}
			if (note.noteFlags & gp_note_has_independent_duration) {
				note.duration = (NoteDuration)gp_read::read_signedbyte(fileStream);
				note.tupletDivision = gp_read::read_signedbyte(fileStream);
			}
			if (note.noteFlags & gp_note_has_dynamics) {
				note.dynamic = gp_read::read_signedbyte(fileStream);
			}
			if (note.noteFlags & gp_note_has_fret) {
				note.fretNumber = gp_read::read_signedbyte(fileStream);
			}
			if (note.noteFlags & gp_note_has_fingering) {
				note.leftHandFinger = gp_read::read_signedbyte(fileStream);
				note.rightHandFinger = gp_read::read_signedbyte(fileStream);
			}
			if (note.noteFlags & gp_note_has_effects) {
				note.noteEffectFlags = gp_read::read_byte(fileStream);
				
				if (note.noteEffectFlags & gp_notefx_bend) {
					note.noteBend = read_bend(fileStream);
				}
				if (note.noteEffectFlags & gp_notefx_grace_note) {
					note.grace = read_grace_note(fileStream);
				}
			}
			
			return note;
		}
		
		Bend read_bend(std::ifstream &fileStream) {
			Bend bend;
			
			bend.type = (BendType)gp_read::read_signedbyte(fileStream);
			bend.value = gp_read::read_int(fileStream);
			bend.pointCount = gp_read::read_int(fileStream);
			
			for (int i = 0; i < bend.pointCount; i++) {
				BendPoint point;
				point.position = gp_read::read_int(fileStream);
				point.value = gp_read::read_int(fileStream);
				point.vibrato = gp_read::read_bool(fileStream);
				bend.points.push_back(point);
			}
			
			return bend;
		}

		GraceNote read_grace_note(std::ifstream &fileStream) {
			GraceNote graceNote;
			
			graceNote.fret = gp_read::read_signedbyte(fileStream);
			graceNote.dynamic = gp_read::read_byte(fileStream);
			graceNote.duration = gp_read::read_byte(fileStream);
			graceNote.transition = gp_read::read_byte(fileStream);
			
			return graceNote;
		}
};