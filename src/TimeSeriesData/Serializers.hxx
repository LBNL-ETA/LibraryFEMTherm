#pragma once

#include <fileParse/Enum.hxx>
#include <fileParse/Optional.hxx>
#include <fileParse/Vector.hxx>

#include "TimeSeriesData.hxx"
#include "Tags.hxx"

namespace TimeSeriesLibrary
{
    //! Constrained to real node adapters so the templates never match std::ostream (which
    //! googletest probes when printing enum values in assertion messages).
    template<typename NodeAdapter>
    concept SerializerNode = requires(NodeAdapter node) { node.addText(std::string_view{}); };

    template<typename NodeAdapter>
        requires SerializerNode<NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ChannelRole & role)
    {
        FileParse::deserializeEnum<NodeAdapter, ChannelRole>(node, role, channelRoleFromString);
        return node;
    }

    template<typename NodeAdapter>
        requires SerializerNode<NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ChannelRole & role)
    {
        FileParse::serializeEnum<NodeAdapter, ChannelRole>(node, role, channelRoleToString);
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Channel & channel)
    {
        Tags tag;
        node >> FileParse::Child{tag.role(), channel.role};
        node >> FileParse::Child{tag.value(), channel.values};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Channel & channel)
    {
        Tags tag;
        node << FileParse::Child{tag.role(), channel.role};
        node << FileParse::Child{tag.value(), channel.values};
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
        node >> FileParse::Child{tag.channel(), data.channels};
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
        node << FileParse::Child{tag.channel(), data.channels};
        return node;
    }
}   // namespace TimeSeriesLibrary
