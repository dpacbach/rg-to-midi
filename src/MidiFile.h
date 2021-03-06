/*
  Rosegarden
  A sequencer and musical notation editor.
  Copyright 2000-2018 the Rosegarden development team.
  See the AUTHORS file for more details.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.  See the file
  COPYING included with this distribution for more information.
*/

#ifndef RG_MIDIFILE_H
#define RG_MIDIFILE_H

#include "Composition.h"
#include "Track.h"
#include "RosegardenDocument.h"

#include <fstream>
#include <string>
#include <vector>
#include <map>

namespace Rosegarden
{

class Composition;
class MidiEvent;

/// Conversion class for Composition to and from MIDI Files.
class MidiFile
{
public:
    MidiFile();
    ~MidiFile();

    /// Convert a Rosegarden composition to a MIDI file.
    /*
     * Returns true on success.
     *
     * See RosegardenMainWindow::exportMIDIFile().
     */
    bool convertToMidi(Composition &, std::string const& filename);
    bool convertToMidi(RosegardenDocument &, std::string const& filename);

private:
    // convertToMidi() uses MidiInserter.
    // MidiInserter uses:
    // - m_timingDivision
    // - m_format
    // - m_numberOfTracks
    // - m_midiComposition
    // - clearMidiComposition()
    friend class MidiInserter;

    // *** Standard MIDI File Header

    enum FileFormatType {
        MIDI_SINGLE_TRACK_FILE          = 0x00,  // Format 0
        MIDI_SIMULTANEOUS_TRACK_FILE    = 0x01,  // Format 1
        MIDI_SEQUENTIAL_TRACK_FILE      = 0x02,  // Format 2
        MIDI_CONVERTED_TO_APPLICATION   = 0xFE,
        MIDI_FILE_NOT_LOADED            = 0xFF
    };
    FileFormatType m_format;

    unsigned m_numberOfTracks;

    enum TimingFormat {
        MIDI_TIMING_PPQ_TIMEBASE,
        MIDI_TIMING_SMPTE
    };
    TimingFormat m_timingFormat;

    /// Pulses Per Quarter note.  (m_timingFormat == MIDI_TIMING_PPQ_TIMEBASE)
    int m_timingDivision;

    // For SMPTE.
    int m_fps;
    int m_subframes;

    // *** Internal MIDI Composition

    /**
     * Our internal MIDI composition is just a vector of MidiEvents.
     * We use a vector and not a set because we want the order of
     * the events to be arbitrary until we explicitly sort them
     * (necessary when converting Composition absolute times to
     * MIDI delta times).
     */
    typedef std::vector<MidiEvent *> MidiTrack;
    typedef std::map<TrackId, MidiTrack> MidiComposition;
    MidiComposition m_midiComposition;
    void clearMidiComposition();

    // *** Standard MIDI File to Rosegarden

    /// Read a MIDI file into m_midiComposition.
    bool read(std::string const& filename);
    void parseHeader(std::ifstream *midiFile);
    /// Convert a track to events in m_midiComposition.
    void parseTrack(std::ifstream *midiFile);
    // m_midiComposition track to MIDI channel.
    std::map<TrackId, int /*channel*/> m_trackChannelMap;
    // Names for each track.
    std::vector<std::string> m_trackNames;
    /// Find the next track chunk and set m_trackByteCount.
    void findNextTrack(std::ifstream *midiFile);
    /// Combine each note-on/note-off pair into a single note event with a duration.
    void consolidateNoteEvents(TrackId trackId);
    /// Configure the Instrument based on events in Segment at time 0.
    //static void configureInstrument(
    //        Track *track, Segment *segment, Instrument *instrument);

    // Read
    /// Read a "variable-length quantity" from the MIDI byte stream.
    /**
     * In case the first byte has already been read, it can be sent
     * in as firstByte.
     */
    long readNumber(std::ifstream *midiFile, int firstByte = -1);
    MidiByte read(std::ifstream *midiFile);
    std::string read(std::ifstream *midiFile,
                     unsigned long numberOfBytes);

    // Conversion
    int midiBytesToInt(const std::string &bytes);
    long midiBytesToLong(const std::string &bytes);

    size_t m_fileSize;
    /// Number of bytes left to read in the current track.
    long m_trackByteCount;
    /// Allow decrementing of m_trackByteCount while reading.
    /**
     * Set to true while processing a track.  false while processing other
     * parts of the file.
     */
    bool m_decrementCount;

    /// Counter for kicking the message loop.
    int m_bytesRead;

    std::string m_error;

    // *** Rosegarden to Standard MIDI File

    /// Write m_midiComposition to a MIDI file.
    bool write(std::string const& filename);
    void writeHeader(std::ofstream *midiFile);
    void writeTrack(std::ofstream *midiFile, TrackId trackNumber);

    // Write
    /// Write an int as 2 bytes.
    void writeInt(std::ofstream *midiFile, int number);
    /// Write a long as 4 bytes.
    void writeLong(std::ofstream *midiFile, unsigned long number);

    // Conversion
    /// Convert a value to a "variable-length quantity" in a std::string.
    std::string longToVarBuffer(unsigned long value);
};


}

#endif // RG_MIDIFILE_H
