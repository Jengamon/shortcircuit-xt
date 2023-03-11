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

#include "sample.h"
#include "infrastructure/file_map_view.h"
#include "dsp/resampling.h"
#include "sst/basic-blocks/mechanics/endian-ops.h"

namespace scxt::sample
{

// Fine in a cpp
using namespace sst::basic_blocks::mechanics;

bool Sample::load(const fs::path &path)
{
    if (!fs::exists(path))
        return false;

    auto extension = path.extension();
    if (extension.compare(".wav") == 0)
    {
        auto fmv = std::make_unique<infrastructure::FileMapView>(path);
        auto data = fmv->data();
        auto datasize = fmv->dataSize();

        clear_data(); // clear to a more predictable state

        bool r = parse_riff_wave(data, datasize);
        // TODO deal with return value
        sample_loaded = true;
        mFileName = path;
        displayName = fmt::format("{}", path.filename().u8string());
        return true;
    }

    return false;
}

bool Sample::loadFromSF2(const fs::path &p, sf2::File *f, int inst, int reg)
{
    mFileName = p;
    instrument = inst;
    region = reg;
    type = SF2_FILE;
    auto sfsample = f->GetInstrument(inst)->GetRegion(region)->GetSample();

    UseInt16 = sfsample->GetFrameSize() == 2;
    channels = sfsample->GetChannelCount();
    sample_length = sfsample->GetTotalFrameCount();
    sample_rate = sfsample->SampleRate;

    auto s = sfsample;

    auto fnp = fs::path{f->GetRiffFile()->GetFileName()};
    displayName = fmt::format("{} ({}/{}/{})", s->Name, fnp.filename().u8string(), inst, region);

    if (!UseInt16)
        return false;

    if (sfsample->GetChannelCount() != 1)
        return false;

    auto buf = sfsample->LoadSampleData();
    load_data_i16(0, buf.pStart, buf.Size, sfsample->GetFrameSize());
    sfsample->ReleaseSampleData();
    return true;
}

// TODO: Rename these
short *Sample::GetSamplePtrI16(int Channel)
{
    if (!UseInt16)
        return 0;
    return &((short *)sampleData[Channel])[scxt::dsp::FIRoffset];
}
float *Sample::GetSamplePtrF32(int Channel)
{
    if (UseInt16)
        return 0;
    return &((float *)sampleData[Channel])[scxt::dsp::FIRoffset];
}

// TODO: What the heck is this doing?
bool Sample::allocateI16(int Channel, int Samples)
{
    // int samplesizewithmargin = Samples + 2*scxt::dsp::FIRipol_N + BLOCK_SIZE +
    // scxt::dsp::FIRoffset;
    int samplesizewithmargin = Samples + scxt::dsp::FIRipol_N;
    if (sampleData[Channel])
        free(sampleData[Channel]);
    sampleData[Channel] = malloc(sizeof(short) * samplesizewithmargin);
    if (!sampleData[Channel])
        return false;
    UseInt16 = true;

    // clear pre/post zero area
    memset(sampleData[Channel], 0, scxt::dsp::FIRoffset * sizeof(short));
    memset((char *)sampleData[Channel] + (Samples + scxt::dsp::FIRoffset) * sizeof(short), 0,
           scxt::dsp::FIRoffset * sizeof(short));

    return true;
}
bool Sample::allocateF32(int Channel, int Samples)
{
    int samplesizewithmargin = Samples + scxt::dsp::FIRipol_N;
    if (sampleData[Channel])
        free(sampleData[Channel]);
    sampleData[Channel] = malloc(sizeof(float) * samplesizewithmargin);
    if (!sampleData[Channel])
        return false;
    UseInt16 = false;

    // clear pre/post zero area
    memset(sampleData[Channel], 0, scxt::dsp::FIRoffset * sizeof(float));
    memset((char *)sampleData[Channel] + (Samples + scxt::dsp::FIRoffset) * sizeof(float), 0,
           scxt::dsp::FIRoffset * sizeof(float));

    return true;
}

bool Sample::load_data_ui8(int channel, void *data, unsigned int samplesize, unsigned int stride)
{
    allocateI16(channel, samplesize);
    short *sampledata = GetSamplePtrI16(channel);

    for (int i = 0; i < samplesize; i++)
    {
        sampledata[i] = (((short)*((unsigned char *)data + i * stride)) - 128) << 8;
    }
    return true;
}

bool Sample::load_data_i8(int channel, void *data, unsigned int samplesize, unsigned int stride)
{
    allocateI16(channel, samplesize);
    short *sampledata = GetSamplePtrI16(channel);

    for (int i = 0; i < samplesize; i++)
    {
        sampledata[i] = ((short)*((char *)data + i * stride)) << 8;
    }
    return true;
}

bool Sample::load_data_i16(int channel, void *data, unsigned int samplesize, unsigned int stride)
{
    allocateI16(channel, samplesize);
    short *sampledata = GetSamplePtrI16(channel);

    for (int i = 0; i < samplesize; i++)
    {
        sampledata[i] = endian_read_int16LE(*(short *)((char *)data + i * stride));
    }
    return true;
}

bool Sample::load_data_i16BE(int channel, void *data, unsigned int samplesize, unsigned int stride)
{
    allocateI16(channel, samplesize);
    short *sampledata = GetSamplePtrI16(channel);

    for (int i = 0; i < samplesize; i++)
    {
        sampledata[i] = endian_read_int16BE(*(short *)((char *)data + i * stride));
    }
    return true;
}
bool Sample::load_data_i32(int channel, void *data, unsigned int samplesize, unsigned int stride)
{
    allocateF32(channel, samplesize);
    float *sampledata = GetSamplePtrF32(channel);

    for (int i = 0; i < samplesize; i++)
    {
        int x = endian_read_int32LE(*(int *)((char *)data + i * stride));
        sampledata[i] = (4.6566128730772E-10f) * (float)x;
    }
    return true;
}

bool Sample::load_data_i32BE(int channel, void *data, unsigned int samplesize, unsigned int stride)
{
    allocateF32(channel, samplesize);
    float *sampledata = GetSamplePtrF32(channel);

    for (int i = 0; i < samplesize; i++)
    {
        int x = endian_read_int32BE(*(int *)((char *)data + i * stride));
        sampledata[i] = (4.6566128730772E-10f) * (float)x;
    }
    return true;
}

bool Sample::load_data_i24(int channel, void *data, unsigned int samplesize, unsigned int stride)
{
    allocateF32(channel, samplesize);
    float *sampledata = GetSamplePtrF32(channel);

    for (int i = 0; i < samplesize; i++)
    {
        unsigned char *cval = (unsigned char *)data + i * stride;
        int value = (cval[2] << 16) | (cval[1] << 8) | cval[0];
        value -= (value & 0x800000) << 1;
        sampledata[i] = 0.00000011920928955078f * float(value);
        ;
    }
    return true;
}

bool Sample::load_data_i24BE(int channel, void *data, unsigned int samplesize, unsigned int stride)
{
    allocateF32(channel, samplesize);
    float *sampledata = GetSamplePtrF32(channel);

    for (int i = 0; i < samplesize; i++)
    {
        unsigned char *cval = (unsigned char *)data + i * stride;
        int value = (cval[0] << 16) | (cval[1] << 8) | cval[2];
        value -= (value & 0x800000) << 1;
        sampledata[i] = 0.00000011920928955078f * float(value);
        ;
    }
    return true;
}

bool Sample::load_data_f32(int channel, void *data, unsigned int samplesize, unsigned int stride)
{
    allocateF32(channel, samplesize);
    float *sampledata = GetSamplePtrF32(channel);

    for (int i = 0; i < samplesize; i++)
    {
        sampledata[i] = (*(float *)((char *)data + i * stride));
    }
    return true;
}

bool Sample::load_data_f64(int channel, void *data, unsigned int samplesize, unsigned int stride)
{
    allocateF32(channel, samplesize);
    float *sampledata = GetSamplePtrF32(channel);

    for (int i = 0; i < samplesize; i++)
    {
        sampledata[i] = (float)(*(double *)((char *)data + i * stride));
    }
    return true;
}

bool Sample::SetMeta(unsigned int Channels, unsigned int SampleRate, unsigned int SampleLength)
{
    if (Channels > 2)
        return false; // not supported

    channels = Channels;
    sample_length = SampleLength;
    sample_rate = SampleRate;
    InvSampleRate = 1.f / (float)SampleRate;

    return true;
}
} // namespace scxt::sample