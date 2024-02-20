/*
 * Shortcircuit XT - a Surge Synth Team product
 *
 * A fully featured creative sampler, available as a standalone
 * and plugin for multiple platforms.
 *
 * Copyright 2019 - 2024, Various authors, as described in the github
 * transaction log.
 *
 * ShortcircuitXT is released under the Gnu General Public Licence
 * V3 or later (GPL-3.0-or-later). The license is found in the file
 * "LICENSE" in the root of this repository or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Individual sections of code which comprises ShortcircuitXT in this
 * repository may also be used under an MIT license. Please see the
 * section  "Licensing" in "README.md" for details.
 *
 * ShortcircuitXT is inspired by, and shares code with, the
 * commercial product Shortcircuit 1 and 2, released by VemberTech
 * in the mid 2000s. The code for Shortcircuit 2 was opensourced in
 * 2020 at the outset of this project.
 *
 * All source for ShortcircuitXT is available at
 * https://github.com/surge-synthesizer/shortcircuit-xt
 */
#ifndef SCXT_SRC_ENGINE_ZONE_H
#define SCXT_SRC_ENGINE_ZONE_H

#include <array>

#include "configuration.h"
#include "utils.h"

#include "keyboard.h"

#include "sst/basic-blocks/dsp/Lag.h"
#include "sample/sample_manager.h"
#include "dsp/processor/processor.h"
#include "modulation/voice_matrix.h"
#include "modulation/modulator_storage.h"

#include "group_and_zone.h"

#include <fmt/core.h>
#include "dsp/generator.h"
#include "bus.h"

namespace scxt::voice
{
struct Voice;
}

namespace scxt::engine
{
struct Group;
struct Engine;

constexpr int lfosPerZone{scxt::lfosPerZone};

struct Zone : MoveableOnly<Zone>, HasGroupZoneProcessors<Zone>, SampleRateSupport
{
    static constexpr int maxSamplesPerZone{scxt::maxSamplesPerZone};
    Zone() : id(ZoneID::next()) { initialize(); }
    Zone(SampleID sid) : id(ZoneID::next())
    {
        sampleData[0].sampleID = sid;
        sampleData[0].active = true;
        initialize();
    }
    Zone(Zone &&) = default;

    ZoneID id;

    enum PlayMode
    {
        NORMAL,     // AEG gates; play on note on
        ONE_SHOT,   // SAMPLE playback gates; play on note on
        ON_RELEASE, // SAMPLE playback gates. play on note off
    };
    DECLARE_ENUM_STRING(PlayMode);

    enum LoopMode
    {
        LOOP_DURING_VOICE, // If a loop begins, stay in it for the life of teh voice
        LOOP_WHILE_GATED,  // If a loop begins, loop while gated
        LOOP_FOR_COUNT     // Loop exactly (n) times
    };
    DECLARE_ENUM_STRING(LoopMode);

    enum LoopDirection
    {
        FORWARD_ONLY,
        ALTERNATE_DIRECTIONS
    };
    DECLARE_ENUM_STRING(LoopDirection);

    struct AssociatedSample
    {
        bool active{false};
        SampleID sampleID;
        int64_t startSample{-1}, endSample{-1}, startLoop{-1}, endLoop{-1};

        PlayMode playMode{NORMAL};
        bool loopActive{false};
        bool playReverse{false};
        LoopMode loopMode{LOOP_DURING_VOICE};
        LoopDirection loopDirection{FORWARD_ONLY};
        int loopCountWhenCounted{0};

        int64_t loopFade{0};

        bool operator==(const AssociatedSample &other) const
        {
            return active == other.active && sampleID == other.sampleID &&
                   startSample == other.startSample && endSample == other.endSample &&
                   startLoop == other.startLoop && endLoop == other.endLoop;
        }
    };
    typedef std::array<AssociatedSample, maxSamplesPerZone> AssociatedSampleArray;
    AssociatedSampleArray sampleData;
    std::array<std::shared_ptr<sample::Sample>, maxSamplesPerZone> samplePointers;

    struct ZoneOutputInfo
    {
        float amplitude{1.f}, pan{0.f};
        bool muted{false};
        ProcRoutingPath procRouting{procRoute_linear};
        BusAddress routeTo{DEFAULT_BUS};
    } outputInfo;
    static_assert(std::is_standard_layout<ZoneOutputInfo>::value);

    float output alignas(16)[2][blockSize];
    void process(Engine &onto);

    // TODO: editable name
    std::string getName() const
    {
        if (samplePointers[0])
            return samplePointers[0]->getDisplayName();
        return id.to_string();
    }

    // TODO: Multi-output
    size_t getNumOutputs() const { return 1; }

    // If this is TRUE then sample root notes, ranges, etc... will override the mapping
    bool sampleLoadOverridesMapping{true};
    bool attachToSample(const sample::SampleManager &manager, int index = 0);

    struct ZoneMappingData
    {
        int16_t rootKey{60};
        KeyboardRange keyboardRange;
        VelocityRange velocityRange;

        int16_t pbDown{2}, pbUp{2};

        int16_t exclusiveGroup;

        float velocitySens{1.0};
        float amplitude{1.0};   // linear
        float pan{0.0};         // -1..1
        float pitchOffset{0.0}; // semitones/keys

    } mapping;

    Group *parentGroup{nullptr};

    bool isActive() { return activeVoices != 0; }
    uint32_t activeVoices{0};
    std::array<voice::Voice *, maxVoices> voiceWeakPointers;
    int gatedVoiceCount{0};

    void initialize();
    // Just a weak ref - don't take ownership. engine manages lifetime
    void addVoice(voice::Voice *);
    void removeVoice(voice::Voice *);

    voice::modulation::Matrix::RoutingTable routingTable;
    std::array<modulation::ModulatorStorage, lfosPerZone> modulatorStorage;

    // 0 is the AEG, 1 is EG2
    std::array<modulation::modulators::AdsrStorage, 2> egStorage;

    void onProcessorTypeChanged(int, dsp::processor::ProcessorType) {}

    void setupOnUnstream(const engine::Engine &e);
    engine::Engine *getEngine();

    sst::basic_blocks::dsp::UIComponentLagHandler mUILag;
    void onSampleRateChanged() override;
};
} // namespace scxt::engine

SC_DESCRIBE(scxt::engine::Zone::ZoneOutputInfo,
            SC_FIELD(amplitude, pmd().asCubicDecibelAttenuation().withName("Amplitude"));
            SC_FIELD(pan, pmd().asPercentBipolar().withName("Pan"));)

SC_DESCRIBE(
    scxt::engine::Zone::ZoneMappingData, SC_FIELD(rootKey, pmd().asMIDINote().withName("Root Key"));
    SC_FIELD(keyboardRange.keyStart, pmd().asMIDINote().withName("Key Start"));
    SC_FIELD(keyboardRange.keyEnd, pmd().asMIDINote().withName("Key End"));
    SC_FIELD(keyboardRange.fadeStart, pmd().asMIDIPitch().withUnit("").withName("Fade Start"));
    SC_FIELD(keyboardRange.fadeEnd, pmd().asMIDIPitch().withUnit("").withName("Fade End"));
    SC_FIELD(velocityRange.velStart, pmd().asMIDIPitch().withUnit("").withName("Velocity Start"));
    SC_FIELD(velocityRange.velEnd, pmd().asMIDIPitch().withUnit("").withName("Velocity End"));
    SC_FIELD(velocityRange.fadeStart,
             pmd().asMIDIPitch().withUnit("").withName("Velocity Fade Start"));
    SC_FIELD(velocityRange.fadeEnd, pmd().asMIDIPitch().withUnit("").withName("Velocity Fade End"));
    SC_FIELD(pbDown, pmd().asMIDIPitch().withUnit("").withDefault(2).withName("Pitch Bend Down"));
    SC_FIELD(pbUp, pmd().asMIDIPitch().withUnit("").withDefault(2).withName("Pitch Bend Up"));
    SC_FIELD(amplitude, pmd().asPercent().withName("Amplitude").withDefault(1.0));
    SC_FIELD(pan, pmd().asPercentBipolar().withName("Pan").withDefault(0.0));
    SC_FIELD(pitchOffset, pmd().asSemitoneRange().withName("Pitch").withDefault(0.0)););

#endif