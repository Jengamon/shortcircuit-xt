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

#ifndef SCXT_SRC_UI_CONNECTORS_PAYLOADDATAATTACHMENT_H
#define SCXT_SRC_UI_CONNECTORS_PAYLOADDATAATTACHMENT_H

#include <functional>
#include <utility>
#include <memory>
#include <string>
#include <type_traits>
#include "sst/jucegui/data/Continuous.h"
#include "sst/jucegui/data/Discrete.h"
#include "datamodel/parameter.h"
#include "sample/sample.h"
#include "components/HasEditor.h"

namespace scxt::ui::connectors
{
// TODO Factor this better obviously
template <typename Payload, typename ValueType = float>
struct PayloadDataAttachment : sst::jucegui::data::Continuous
{
    typedef Payload payload_t;
    typedef ValueType value_t;

    ValueType &value;
    std::string label;
    std::function<void(const PayloadDataAttachment &at)> onGuiValueChanged;

    PayloadDataAttachment(const datamodel::pmd &cd,
                          std::function<void(const PayloadDataAttachment &at)> oGVC, ValueType &v)
        : description(cd), value(v), label(cd.name), onGuiValueChanged(std::move(oGVC))
    {
    }

    PayloadDataAttachment(const datamodel::pmd &cd, ValueType &v)
        : description(cd), value(v), label(cd.name), onGuiValueChanged(nullptr)
    {
    }

    template <typename M> void asFloatUpdate(const Payload &p, HasEditor *e)
    {
        static_assert(std::is_standard_layout_v<Payload>);

        ptrdiff_t pdiff = (uint8_t *)&value - (uint8_t *)&p;
        assert(pdiff >= 0);
        assert(pdiff <= sizeof(p) - sizeof(value));

        auto jc = dynamic_cast<juce::Component *>(e);
        assert(jc);

        onGuiValueChanged = [w = juce::Component::SafePointer(jc), e,
                             pdiff](const PayloadDataAttachment &a) {
            if (w)
            {
                e->sendToSerialization(M({pdiff, a.value}));
                e->updateValueTooltip(a);
            }
        };
    }

    template <typename M> void asFloatUpdate(const Payload &p, size_t &index, HasEditor *e)
    {
        static_assert(std::is_standard_layout_v<Payload>);

        ptrdiff_t pdiff = (uint8_t *)&value - (uint8_t *)&p;
        assert(pdiff >= 0);
        assert(pdiff <= sizeof(p) - sizeof(value));

        auto jc = dynamic_cast<juce::Component *>(e);
        assert(jc);

        onGuiValueChanged = [w = juce::Component::SafePointer(jc), e, &index,
                             pdiff](const PayloadDataAttachment &a) {
            if (w)
            {
                e->sendToSerialization(M({index, pdiff, a.value}));
                e->updateValueTooltip(a);
            }
        };
    }

    PayloadDataAttachment(const PayloadDataAttachment &other) = delete;
    PayloadDataAttachment &operator=(const PayloadDataAttachment &other) = delete;
    PayloadDataAttachment &operator=(PayloadDataAttachment &&other) = delete;

    // TODO maybe. For now we have these descriptions as constexpr
    // in the code and apply them directly in many cases. We could
    // stream each and every one but don't. Maybe fix that one day.
    // Would def need to fix that to make web ui consistent if we write it
    datamodel::pmd description;

    std::string getLabel() const override { return label; }
    float getValue() const override { return (float)value; }
    void setValueFromGUI(const float &f) override
    {
        value = (ValueType)f;
        if (onGuiValueChanged)
        {
            onGuiValueChanged(*this);
        }
    }

    std::function<std::string(float)> valueToString{nullptr};
    std::string getValueAsStringFor(float f) const override
    {
        if (description.supportsStringConversion)
        {
            auto res = description.valueToString(f);
            if (res.has_value())
                return *res;
        }
        if (valueToString)
            return valueToString(f);
        return Continuous::getValueAsStringFor(f);
    }
    std::function<std::optional<float>(const std::string &)> stringToValue{nullptr};
    void setValueAsString(const std::string &s) override
    {
        if (description.supportsStringConversion)
        {
            std::string em;
            auto res = description.valueFromString(s, em);
            if (res.has_value())
            {
                setValueFromGUI(*res);
                return;
            }
        }
        if (stringToValue)
        {
            auto f = stringToValue(s);
            if (f.has_value())
            {
                setValueFromGUI(*f);
                return;
            }
        }
        Continuous::setValueAsString(s);
    }
    void setValueFromModel(const float &f) override { value = (ValueType)f; }

    float getMin() const override { return description.minVal; }
    float getMax() const override { return description.maxVal; }
    float getDefaultValue() const override { return description.defaultVal; }

    bool isBipolar() const override { return description.isBipolar(); }
};

template <typename Payload, typename ValueType = int>
struct DiscretePayloadDataAttachment : sst::jucegui::data::Discrete
{
    ValueType &value;
    std::string label;
    std::function<void(const DiscretePayloadDataAttachment &at)> onGuiValueChanged;

    DiscretePayloadDataAttachment(const datamodel::pmd &cd,
                                  std::function<void(const DiscretePayloadDataAttachment &at)> oGVC,
                                  ValueType &v)
        : description(cd), value(v), label(cd.name), onGuiValueChanged(std::move(oGVC))
    {
    }

    // TODO maybe. For now we have these descriptions as constexpr
    // in the code and apply them directly in many cases. We could
    // stream each and every one but don't. Maybe fix that one day.
    // Would def need to fix that to make web ui consistent if we write it
    datamodel::pmd description;

    std::string getLabel() const override { return label; }
    int getValue() const override { return (int)value; }
    void setValueFromGUI(const int &f) override
    {
        value = (ValueType)f;
        onGuiValueChanged(*this);
    }
    void setValueFromModel(const int &f) override { value = (ValueType)f; }

    int getMin() const override { return (int)description.minVal; }
    int getMax() const override { return (int)description.maxVal; }

    std::string getValueAsStringFor(int i) const override
    {
        auto r = description.valueToString((float)i);
        if (r.has_value())
            return *r;
        return "";
    }
};

template <typename Payload>
struct BooleanPayloadDataAttachment : DiscretePayloadDataAttachment<Payload, bool>
{
    BooleanPayloadDataAttachment(
        const std::string &l,
        std::function<void(const DiscretePayloadDataAttachment<Payload, bool> &at)> oGVC, bool &v)
        : DiscretePayloadDataAttachment<Payload, bool>(
              datamodel::pmd().withType(datamodel::pmd::BOOL).withName(l), oGVC, v)
    {
    }

    int getMin() const override { return (int)0; }
    int getMax() const override { return (int)1; }

    std::string getValueAsStringFor(int i) const override { return i == 0 ? "Off" : "On"; }
};

struct DirectBooleanPayloadDataAttachment : sst::jucegui::data::Discrete
{
    bool &value;
    std::function<void(const bool)> callback;
    DirectBooleanPayloadDataAttachment(std::function<void(const bool val)> oGVC, bool &v)
        : callback(oGVC), value(v)
    {
    }

    std::string getLabel() const override { return "Bool"; }
    int getValue() const override { return value ? 1 : 0; }
    void setValueFromGUI(const int &f) override
    {
        value = f;
        callback(f);
    }
    void setValueFromModel(const int &f) override { value = f; }

    int getMin() const override { return (int)0; }
    int getMax() const override { return (int)1; }

    std::string getValueAsStringFor(int i) const override { return i == 0 ? "Off" : "On"; }
};

struct SamplePointDataAttachment : sst::jucegui::data::Continuous
{
    int64_t &value;
    std::string label;
    int64_t sampleCount{0};
    std::function<void(const SamplePointDataAttachment &)> onGuiChanged{nullptr};

    SamplePointDataAttachment(int64_t &v,
                              std::function<void(const SamplePointDataAttachment &)> ogc)
        : value(v), onGuiChanged(ogc)
    {
    }

    float getValue() const override { return value; }
    std::string getValueAsStringFor(float f) const override
    {
        if (f < 0)
            return "";
        return fmt::format("{}", (int64_t)f);
    }
    void setValueFromGUI(const float &f) override
    {
        value = (int64_t)f;
        if (onGuiChanged)
            onGuiChanged(*this);
    }
    std::string getLabel() const override { return label; }
    float getQuantizedStepSize() const override { return 1; }
    float getMin() const override { return -1; }
    float getMax() const override { return sampleCount; }
    float getDefaultValue() const override { return 0; }
    void setValueFromModel(const float &f) override { value = (int64_t)f; }
};

template <typename A, typename Msg> struct SingleValueFactory
{
    template <typename W>
    static std::pair<std::unique_ptr<A>, std::unique_ptr<W>>
    attachR(const datamodel::pmd &md, const typename A::payload_t &p, typename A::value_t &val,
            HasEditor *e)
    {
        auto att = std::make_unique<A>(md, val);
        att->template asFloatUpdate<Msg>(p, e);
        auto wid = std::make_unique<W>();
        wid->setSource(att.get());
        e->setupWidgetForValueTooltip(wid.get(), att.get());

        return {std::move(att), std::move(wid)};
    }

    template <typename W>
    static void attach(const datamodel::pmd &md, const typename A::payload_t &p,
                       typename A::value_t &val, HasEditor *e, std::unique_ptr<A> &aRes,
                       std::unique_ptr<W> &wRes)
    {
        auto [a, w] = attachR<W>(md, p, val, e);
        aRes = std::move(a);
        wRes = std::move(w);
    }

    template <typename W>
    static void attachAndAdd(const datamodel::pmd &md, const typename A::payload_t &p,
                             typename A::value_t &val, HasEditor *e, std::unique_ptr<A> &aRes,
                             std::unique_ptr<W> &wRes)
    {
        auto [a, w] = attachR<W>(md, p, val, e);
        aRes = std::move(a);
        wRes = std::move(w);
        auto jc = dynamic_cast<juce::Component *>(e);
        assert(jc);
        if (jc)
        {
            jc->addAndMakeVisible(*wRes);
        }
    }
};

template <typename A, typename Msg> struct SingleValueIndexedFactory
{
    template <typename W>
    static std::pair<std::unique_ptr<A>, std::unique_ptr<W>>
    attachR(const datamodel::pmd &md, const typename A::payload_t &p, size_t &index,
            typename A::value_t &val, HasEditor *e)
    {
        auto att = std::make_unique<A>(md, val);
        att->template asFloatUpdate<Msg>(p, index, e);
        auto wid = std::make_unique<W>();
        wid->setSource(att.get());
        e->setupWidgetForValueTooltip(wid.get(), att.get());

        return {std::move(att), std::move(wid)};
    }

    template <typename W>
    static void attach(const datamodel::pmd &md, const typename A::payload_t &p, size_t &index,
                       typename A::value_t &val, HasEditor *e, std::unique_ptr<A> &aRes,
                       std::unique_ptr<W> &wRes)
    {
        auto [a, w] = attachR<W>(md, p, index, val, e);
        aRes = std::move(a);
        wRes = std::move(w);
    }

    template <typename W>
    static void attachAndAdd(const datamodel::pmd &md, const typename A::payload_t &p,
                             size_t &index, typename A::value_t &val, HasEditor *e,
                             std::unique_ptr<A> &aRes, std::unique_ptr<W> &wRes)
    {
        auto [a, w] = attachR<W>(md, p, index, val, e);
        aRes = std::move(a);
        wRes = std::move(w);
        auto jc = dynamic_cast<juce::Component *>(e);
        assert(jc);
        if (jc)
        {
            jc->addAndMakeVisible(*wRes);
        }
    }
};
} // namespace scxt::ui::connectors
#endif // SHORTCIRCUIT_PAYLOADDATAATTACHMENT_H
