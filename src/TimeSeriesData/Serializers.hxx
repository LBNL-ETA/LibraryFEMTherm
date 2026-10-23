#pragma once

#include <optional>
#include <string>
#include <vector>

#include <fileParse/Enum.hxx>
#include <fileParse/Optional.hxx>
#include <fileParse/Vector.hxx>

#include "TimeSeriesData.hxx"
#include "Tags.hxx"
#include "ValueText.hxx"

namespace TimeSeriesLibrary
{
    //! Constrained to real node adapters so the templates never match std::ostream (which
    //! googletest probes when printing enum values in assertion messages).
    template<typename NodeAdapter>
    concept SerializerNode = requires(NodeAdapter node) { node.addText(std::string_view{}); };

    template<typename NodeAdapter>
        requires SerializerNode<NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, SeriesRole & role)
    {
        FileParse::deserializeEnum<NodeAdapter, SeriesRole>(node, role, seriesRoleFromString);
        return node;
    }

    template<typename NodeAdapter>
        requires SerializerNode<NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const SeriesRole & role)
    {
        FileParse::serializeEnum<NodeAdapter, SeriesRole>(node, role, seriesRoleToString);
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Series & series)
    {
        Tags tag;
        node >> FileParse::Child{tag.role(), series.role};

        std::string valuesText;
        node >> FileParse::Child{tag.values(), valuesText};
        series.values = parseSeriesValues(valuesText);
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Series & series)
    {
        Tags tag;
        node << FileParse::Child{tag.role(), series.role};
        const std::string valuesText{formatSeriesValues(series.values)};
        node << FileParse::Child{tag.values(), valuesText};
        return node;
    }

    //! A missing TimeAxis element leaves the defaults (1 January 00:00, hourly): datasets
    //! saved before the axis existed keep the meaning they always had. The file states the
    //! steps as StepSeconds, the one length or the first of the list, and StepLengths, the
    //! list as the comma-separated text Series/Values uses, written only when the steps
    //! differ: an evenly stepped axis reads and writes exactly as it did before uneven
    //! steps existed.
    template<typename NodeAdapter>
        requires SerializerNode<NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, TimeAxis & axis)
    {
        Tags tag;
        node >> FileParse::Child{tag.month(), axis.month};
        node >> FileParse::Child{tag.day(), axis.day};
        node >> FileParse::Child{tag.hour(), axis.hour};
        node >> FileParse::Child{tag.minute(), axis.minute};
        double every{stepSecondsAt(axis, 0U)};
        node >> FileParse::Child{tag.stepSeconds(), every};
        std::optional<std::string> lengthsText;
        node >> FileParse::Child{tag.stepLengths(), lengthsText};
        auto lengths{lengthsText.has_value() ? parseSeriesValues(lengthsText.value()) : std::vector<double>{}};
        axis.stepSeconds = lengths.empty() ? StepSeconds{every} : StepSeconds{std::move(lengths)};
        return node;
    }

    template<typename NodeAdapter>
        requires SerializerNode<NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const TimeAxis & axis)
    {
        Tags tag;
        node << FileParse::Child{tag.month(), axis.month};
        node << FileParse::Child{tag.day(), axis.day};
        node << FileParse::Child{tag.hour(), axis.hour};
        node << FileParse::Child{tag.minute(), axis.minute};
        double first{stepSecondsAt(axis, 0U)};
        node << FileParse::Child{tag.stepSeconds(), first};
        if(const auto * lengths{std::get_if<std::vector<double>>(&axis.stepSeconds)})
        {
            const std::string lengthsText{formatSeriesValues(*lengths)};
            node << FileParse::Child{tag.stepLengths(), lengthsText};
        }
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, TimeSeriesData & data)
    {
        Tags tag;
        node >> FileParse::Child{tag.uuid(), data.UUID};
        node >> FileParse::Child{tag.name(), data.Name};
        node >> FileParse::Child{tag.projectName(), data.ProjectName};
        node >> FileParse::Child{tag.protectedTag(), data.Protected};
        node >> FileParse::Child{tag.description(), data.Description};
        node >> FileParse::Child{tag.color(), data.Color};
        node >> FileParse::Child{tag.source(), data.Source};
        node >> FileParse::Child{tag.timeAxis(), data.axis};
        node >> FileParse::Child{tag.series(), data.series};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const TimeSeriesData & data)
    {
        Tags tag;
        node << FileParse::Child{tag.uuid(), data.UUID};
        node << FileParse::Child{tag.name(), data.Name};
        node << FileParse::Child{tag.projectName(), data.ProjectName};
        node << FileParse::Child{tag.protectedTag(), data.Protected};
        node << FileParse::Child{tag.description(), data.Description};
        node << FileParse::Child{tag.color(), data.Color};
        node << FileParse::Child{tag.source(), data.Source};
        node << FileParse::Child{tag.timeAxis(), data.axis};
        node << FileParse::Child{tag.series(), data.series};
        return node;
    }
}   // namespace TimeSeriesLibrary
