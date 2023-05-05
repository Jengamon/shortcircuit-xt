/*
 * Shortcircuit XT - a Surge Synth Team product
 *
 * A fully featured creative sampler, available as a standalone
 * and plugin for multiple platforms.
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
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

#ifndef SCXT_SRC_ENGINE_PART_EFFECTS_H
#define SCXT_SRC_ENGINE_PART_EFFECTS_H

#include <memory>
namespace scxt::engine
{
struct Engine;

struct PartEffectStorage
{
    static constexpr int maxPartEffectParams{12};
    float params[maxPartEffectParams];
};
struct PartEffect
{
    PartEffect(Engine *, PartEffectStorage *, float *) {}
    virtual ~PartEffect() = default;

    virtual void init() = 0;
    virtual void process(float *__restrict L, float *__restrict R) = 0;
};

enum AvailablePartEffects
{
    reverb1,
    flanger
};

std::unique_ptr<PartEffect> createEffect(AvailablePartEffects p, Engine *e, PartEffectStorage *s);
} // namespace scxt::engine

#endif // SHORTCIRCUITXT_PART_FX_H
