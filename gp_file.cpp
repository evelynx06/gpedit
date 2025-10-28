#include <iostream>
#include <vector>
#include <fstream>

#include "gp_file.hpp"
#include "gp_read.hpp"

		
GPFile::GPFile(std::ifstream &fileStream) {
	read_song(fileStream);
}
		
int GPFile::read_song(std::ifstream &fileStream) {
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


int GPFile::read_version(std::ifstream &fileStream) {
	this->version = gp_read::read_bytestring(fileStream);
	fileStream.seekg(30 - this->version.length(), std::ifstream::cur);
	
	if (this->version != "FICHIER GUITAR PRO v3.00") {
		std::cerr << "Incompatible file format '" << this->version << "'\n";
		return 1;
	}
	
	return 0;
}

int GPFile::read_metadata(std::ifstream &fileStream) {
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

int GPFile::read_midi_channels(std::ifstream &fileStream) {
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

MeasureHeader GPFile::read_measure_header(std::ifstream &fileStream) {
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

TrackHeader GPFile::read_track_header(std::ifstream &fileStream) {
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

Measure GPFile::read_measure(std::ifstream &fileStream) {
	Measure measure;
	
	measure.beatCount = gp_read::read_int(fileStream);
	
	for (int i = 0; i < measure.beatCount; i++) {
		measure.beats.push_back(read_beat(fileStream));
	}
	
	return measure;
}

Beat GPFile::read_beat(std::ifstream &fileStream) {
	Beat beat;
	
	beat.beatFlags = gp_read::read_byte(fileStream);
	
	if (beat.beatFlags & gp_beat_is_empty_or_rest) {
		beat.isRest = gp_read::read_bool(fileStream);
	}
	else {
		beat.isRest = 0;
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

Chord GPFile::read_chord(std::ifstream &fileStream) {
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

BeatEffects GPFile::read_beat_effects(std::ifstream &fileStream) {
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

MixChange GPFile::read_mix_change(std::ifstream &fileStream) {
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

Notes GPFile::read_notes(std::ifstream &fileStream) {
	Notes notes;
	
	notes.stringsPlayed = gp_read::read_byte(fileStream);
	
	for (int i = 0; i < 7; i++) {
		if (notes.stringsPlayed & (0x40 >> i)) {
			notes.strings[i] = read_note(fileStream);
		}
	}
	
	return notes;
}

Note GPFile::read_note(std::ifstream &fileStream) {
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

Bend GPFile::read_bend(std::ifstream &fileStream) {
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

GraceNote GPFile::read_grace_note(std::ifstream &fileStream) {
	GraceNote graceNote;
	
	graceNote.fret = gp_read::read_signedbyte(fileStream);
	graceNote.dynamic = gp_read::read_byte(fileStream);
	graceNote.duration = gp_read::read_byte(fileStream);
	graceNote.transition = gp_read::read_byte(fileStream);
	
	return graceNote;
}