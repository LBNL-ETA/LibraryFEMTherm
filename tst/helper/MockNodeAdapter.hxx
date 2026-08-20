#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <fileParse/INodeAdapter.hxx>

namespace Helper
{
    // Intention is to emulate XML or JSON data structure without need to be written into memory
    struct MockNode
    {
        MockNode() = default;

        explicit MockNode(std::string tag);

        std::string tag;
        std::string text;
        std::vector<MockNode> child;

        std::map<std::string, std::string> attributes_;

        MockNode & addChild(const std::string & tagName);

        void addAttribute(std::string_view name, std::string_view value);

        [[nodiscard]] std::optional<std::string> getAttribute(std::string_view name) const;
    };

    [[maybe_unused]] MockNode &
      addChildNode(MockNode & parentNode, std::string_view tag, std::string_view text = "");
    [[maybe_unused]] MockNode &
      addChildNodeIfNotEmpty(MockNode & parentNode, std::string_view tag, std::string_view text = "");
    [[maybe_unused]] MockNode &
      addChildNode(MockNode & parentNode, std::string_view tag, MockNode childNode);
    void addChildNode(MockNode & parentNode, MockNode childNode);

    [[nodiscard]] bool compareNodes(const MockNode & node1, const MockNode & node2);


    struct MockNodeAdapter
    {
        explicit MockNodeAdapter(MockNode * node);

        [[nodiscard]] bool isEmpty() const;
        [[nodiscard]] std::string getCurrentTag() const;
        [[nodiscard]] std::vector<MockNodeAdapter> getChildNodes() const;
        [[nodiscard]] std::optional<MockNodeAdapter>
          getFirstChildByName(std::string_view name) const;
        [[nodiscard]] std::vector<MockNodeAdapter>
          getChildNodesByName(std::string_view name) const;
        [[nodiscard]] int nChildNode(std::string_view name) const;
        [[nodiscard]] bool hasChildNode(std::string_view name) const;
        [[nodiscard]] std::string getText() const;
        MockNodeAdapter addChild(std::string_view name);
        void addText(std::string_view text);

        [[nodiscard]] MockNode getNode() const;

        [[nodiscard]] std::string getContent() const;

        void addAttribute(std::string_view name, std::string_view value);
        [[nodiscard]] std::optional<std::string> getAttribute(std::string_view name) const;

    private:
        MockNode * node_{nullptr};
    };
}   // namespace Helper

// No static_assert against FileParse::NodeAdapterLike here on purpose: the pinned
// FileParse (Version_1.1.5) predates the concept, and the duck-typed serializer
// templates enforce the adapter shape at every instantiation anyway. Add the assert
// back once the pin moves past the concept-introducing FileParse release.