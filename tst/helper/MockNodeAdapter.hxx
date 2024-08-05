#pragma once

#include <string>
#include <vector>
#include <memory>

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

        MockNode & addChild(const std::string & tagName)
        {
            // add child
            child.emplace_back(tagName);
            return child.back();
        }
    };

    [[maybe_unused]] MockNode &
      addChildNode(MockNode & parentNode, std::string_view tag, std::string_view text = "");
    [[maybe_unused]] MockNode &
      addChildNode(MockNode & parentNode, std::string_view tag, MockNode childNode);
    void addChildNode(MockNode & parentNode, MockNode childNode);

    [[nodiscard]] bool compareNodes(const MockNode & node1, const MockNode & node2);


    struct MockNodeAdapter : public INodeAdapter<MockNodeAdapter>
    {
        explicit MockNodeAdapter(MockNode * node);

        [[nodiscard]] bool isEmpty() const override;
        [[nodiscard]] std::string getCurrentTag() const override;
        [[nodiscard]] std::vector<MockNodeAdapter> getChildNodes() const override;
        [[nodiscard]] std::optional<MockNodeAdapter>
          getFirstChildByName(std::string_view name) const override;
        [[nodiscard]] std::vector<MockNodeAdapter>
          getChildNodesByName(std::string_view name) const override;
        [[nodiscard]] int nChildNode(std::string_view name) const override;
        [[nodiscard]] bool hasChildNode(std::string_view name) const override;
        [[nodiscard]] std::string getText() const override;
        MockNodeAdapter addChild(std::string_view name) override;
        void addText(std::string_view text) override;

        [[nodiscard]] MockNode getNode() const;

    private:
        MockNode * node_{nullptr};
    };
}