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

#ifndef SCXT_SRC_DSP_PROCESSOR_PROCESSOR_DEFS_H
#define SCXT_SRC_DSP_PROCESSOR_PROCESSOR_DEFS_H

#include "processor.h"
#include "datamodel/metadata.h"
#include "infrastructure/sse_include.h"

#include <sst/filters/HalfRateFilter.h>
#include "sst/basic-blocks/dsp/BlockInterpolators.h"
#include "configuration.h"

/**
 * Adding a processor here. Used to be we had a bunch of case staements. But now we are all
 * driven by templates (see "processor.cpp" for some hairy template code you never need to touch).
 * So to add a processor
 *
 * 1. Add it in a .c/.hpp as a subtype of Processor
 * 2. Implement these constexpr values
 *       static constexpr const char *processorName{"OSC Pulse"};
 *       static constexpr const char *processorStreamingName{"osc-pulse"};
 *       static constexpr const char *processorDisplayGroup{"Generators"};
 *
 *       The streaming name has to be stable across versions
 *
 * 3. Specialize the id-to-type structure
 *      template <> struct ProcessorImplementor<ProcessorType::proct_osc_pulse_sync>
 *      {
 *          typedef OscPulseSync T;
 *      };
 *
 *  and then include the h here and everything will work
 */

#include "definition_helpers.h"
#include "dsp/processor/processor_impl.h"

#include "sst/voice-effects/distortion/Microgate.h"
#include "sst/voice-effects/distortion/BitCrusher.h"
#include "sst/voice-effects/distortion/Slewer.h"
#include "sst/voice-effects/waveshaper/WaveShaper.h"

#include "sst/voice-effects/eq/EqNBandParametric.h"
#include "sst/voice-effects/eq/EqGraphic6Band.h"
#include "sst/voice-effects/eq/MorphEQ.h"

#include "sst/voice-effects/generator/GenCorrelatedNoise.h"
#include "sst/voice-effects/generator/GenSin.h"
#include "sst/voice-effects/generator/GenSaw.h"
#include "sst/voice-effects/generator/GenPulseSync.h"
#include "sst/voice-effects/generator/GenPhaseMod.h"

#include "sst/voice-effects/pitch/PitchRing.h"

#include "sst/voice-effects/delay/FauxStereo.h"
#include "sst/voice-effects/delay/ShortDelay.h"
#include "sst/voice-effects/delay/StringExciter.h"

#include "sst/voice-effects/filter/CytomicSVF.h"
#include "sst/voice-effects/filter/SurgeBiquads.h"
#include "sst/voice-effects/filter/SSTFilters.h"
#include "sst/voice-effects/filter/StaticPhaser.h"

namespace scxt::dsp::processor
{
// Just don't change the id or streaming name, basically
DEFINE_PROC(MicroGate, sst::voice_effects::distortion::MicroGate<SCXTVFXConfig<1>>,
            sst::voice_effects::distortion::MicroGate<SCXTVFXConfig<2>>, proct_fx_microgate,
            "MicroGate", "Distortion", "micro-gate-fx");
DEFINE_PROC(BitCrusher, sst::voice_effects::distortion::BitCrusher<SCXTVFXConfig<1>>,
            sst::voice_effects::distortion::BitCrusher<SCXTVFXConfig<2>>, proct_fx_bitcrusher,
            "BitCrusher", "Distortion", "bit-crusher-fx");
DEFINE_PROC(WaveShaper, sst::voice_effects::waveshaper::WaveShaper<SCXTVFXConfig<1>>,
            sst::voice_effects::waveshaper::WaveShaper<SCXTVFXConfig<2>>, proct_fx_waveshaper,
            "WaveShaper", "Distortion", "waveshaper-fx");
DEFINE_PROC(Slewer, sst::voice_effects::distortion::Slewer<SCXTVFXConfig<1>>,
            sst::voice_effects::distortion::Slewer<SCXTVFXConfig<2>>, proct_fx_slewer, "Slewer",
            "Distortion", "slewer-fx");

// Macros and commas don't get along
namespace procimpl::detail
{
using eq1impl = sst::voice_effects::eq::EqNBandParametric<SCXTVFXConfig<1>, 1>;
using eq2impl = sst::voice_effects::eq::EqNBandParametric<SCXTVFXConfig<1>, 2>;
using eq3impl = sst::voice_effects::eq::EqNBandParametric<SCXTVFXConfig<1>, 3>;

using eq1impl_os = sst::voice_effects::eq::EqNBandParametric<SCXTVFXConfig<2>, 1>;
using eq2impl_os = sst::voice_effects::eq::EqNBandParametric<SCXTVFXConfig<2>, 2>;
using eq3impl_os = sst::voice_effects::eq::EqNBandParametric<SCXTVFXConfig<2>, 3>;

} // namespace procimpl::detail

DEFINE_PROC(EQ1Band, procimpl::detail::eq1impl, procimpl::detail::eq1impl_os,
            proct_eq_1band_parametric_A, "1 Band Parametric", "EQ", "eq-parm-1band");
DEFINE_PROC(EQ2Band, procimpl::detail::eq2impl, procimpl::detail::eq2impl_os,
            proct_eq_2band_parametric_A, "2 Band Parametric", "EQ", "eq-parm-2band");
DEFINE_PROC(EQ3Band, procimpl::detail::eq3impl, procimpl::detail::eq3impl_os,
            proct_eq_3band_parametric_A, "3 Band Parametric", "EQ", "eq-parm-3band");
DEFINE_PROC(EQGraphic6Band, sst::voice_effects::eq::EqGraphic6Band<SCXTVFXConfig<1>>,
            sst::voice_effects::eq::EqGraphic6Band<SCXTVFXConfig<2>>, proct_eq_6band,
            "6 Band Graphic", "EQ", "eq-grp-6");

DEFINE_PROC(MorphEQ, sst::voice_effects::eq::MorphEQ<SCXTVFXConfig<1>>,
            sst::voice_effects::eq::MorphEQ<SCXTVFXConfig<2>>, proct_eq_morph, "Morph", "EQ",
            "eq-morph");

DEFINE_PROC(GenSin, sst::voice_effects::generator::GenSin<SCXTVFXConfig<1>>,
            sst::voice_effects::generator::GenSin<SCXTVFXConfig<2>>, proct_osc_sin, "Sin",
            "Generators", "osc-sin");
DEFINE_PROC(GenSaw, sst::voice_effects::generator::GenSaw<SCXTVFXConfig<1>>,
            sst::voice_effects::generator::GenSaw<SCXTVFXConfig<2>>, proct_osc_saw, "Saw",
            "Generators", "osc-saw");
DEFINE_PROC(GenPulseSync, sst::voice_effects::generator::GenPulseSync<SCXTVFXConfig<1>>,
            sst::voice_effects::generator::GenPulseSync<SCXTVFXConfig<2>>, proct_osc_pulse_sync,
            "Pulse Sync", "Generators", "osc-pulse-sync", dsp::sincTable);
DEFINE_PROC(GenPhaseMod, sst::voice_effects::generator::GenPhaseMod<SCXTVFXConfig<1>>,
            sst::voice_effects::generator::GenPhaseMod<SCXTVFXConfig<2>>, proct_osc_phasemod,
            "Phase Mod", "Generators", "osc-phase-mod");
DEFINE_PROC(GenCorrelatedNoise, sst::voice_effects::generator::GenCorrelatedNoise<SCXTVFXConfig<1>>,
            sst::voice_effects::generator::GenCorrelatedNoise<SCXTVFXConfig<2>>,
            proct_osc_correlatednoise, "Correlated Noise", "Generators", "osc-correlated-noise");

DEFINE_PROC(PitchRing, sst::voice_effects::pitch::PitchRing<SCXTVFXConfig<1>>,
            sst::voice_effects::pitch::PitchRing<SCXTVFXConfig<2>>, proct_fx_pitchring, "PitchRing",
            "Pitch and Frequency", "pitchring-fx");

DEFINE_PROC(FauxStereo, sst::voice_effects::delay::FauxStereo<SCXTVFXConfig<1>>,
            sst::voice_effects::delay::FauxStereo<SCXTVFXConfig<2>>, proct_fx_fauxstereo,
            "Faux Stereo", "Delay Based", "fxstereo-fx", dsp::surgeSincTable);
DEFINE_PROC(ShortDelay, sst::voice_effects::delay::ShortDelay<SCXTVFXConfig<1>>,
            sst::voice_effects::delay::ShortDelay<SCXTVFXConfig<2>>, proct_fx_simple_delay,
            "Simple Delay", "Delay Based", "simpdel-fx", dsp::surgeSincTable);
DEFINE_PROC(StringExciter, sst::voice_effects::delay::StringExciter<SCXTVFXConfig<1>>,
            sst::voice_effects::delay::StringExciter<SCXTVFXConfig<2>>, proct_StringExciter,
            "String Exciter", "Delay Based", "stringex-fx", dsp::surgeSincTable);

DEFINE_PROC(CytomicSVF, sst::voice_effects::filter::CytomicSVF<SCXTVFXConfig<1>>,
            sst::voice_effects::filter::CytomicSVF<SCXTVFXConfig<2>>, proct_CytomicSVF, "Fast SVF",
            "Filters", "filt-cytomic");

DEFINE_PROC(StaticPhaser, sst::voice_effects::filter::StaticPhaser<SCXTVFXConfig<1>>,
            sst::voice_effects::filter::StaticPhaser<SCXTVFXConfig<2>>, proct_StaticPhaser,
            "Static Phaser", "Filters", "filt-statph");

DEFINE_PROC(SurgeBiquads, sst::voice_effects::filter::SurgeBiquads<SCXTVFXConfig<1>>,
            sst::voice_effects::filter::SurgeBiquads<SCXTVFXConfig<2>>, proct_SurgeBiquads,
            "Surge Biquads", "Filters", "filt-sstbiquad");

DEFINE_PROC(SSTFilters, sst::voice_effects::filter::SSTFilters<SCXTVFXConfig<1>>,
            sst::voice_effects::filter::SSTFilters<SCXTVFXConfig<2>>, proct_SurgeFilters,
            "Surge Filters", "Filters", "filt-sstfilters");

} // namespace scxt::dsp::processor

#endif // __SCXT_PROCESSOR_DEFS_H
