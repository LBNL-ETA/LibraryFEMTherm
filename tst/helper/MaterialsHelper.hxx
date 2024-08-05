// /brief Routines that will be used to help with testing of materials.
// File: MaterialHelper.hpp

#pragma once

#include <optional>
#include <vector>

#include "Materials/Materials.hxx"

namespace Helper
{
    struct MockNode;

    //////////////////////////////////////////////////////////////////////////////////////////
    //// Helper functions to test the data
    //////////////////////////////////////////////////////////////////////////////////////////

    /// \brief Asserts that the given optional vector of points contains the expected values.
    /// \param optionalPoints The optional vector of points to be tested.
    /// \param expectedValues The expected values of the points.
    /// \param tolerance The tolerance for comparing the values.
    void assertVectorOfPoints(const std::vector<std::pair<double, double>> & expectedValues,
                              const std::optional<std::vector<MaterialsLibrary::point>> & optionalPoints,
                              double tolerance);

    /// \brief Asserts that the given optical properties contain the expected values.
    /// \param correct The correct optical properties.
    /// \param calculated The calculated optical properties.
    /// \param tolerance The tolerance for comparing the values.
    void assertOpticalProperties(const MaterialsLibrary::OpticalProperties & correct,
                                 const MaterialsLibrary::OpticalProperties & calculated,
                                 double tolerance);

    /// \brief Asserts that the given optical side contains the expected values.
    /// \param correct The correct optical side.
    /// \param calculated The calculated optical side.
    /// \param tolerance The tolerance for comparing the values.
    void assertOpticalSide(const MaterialsLibrary::OpticalSide & correct,
                           const MaterialsLibrary::OpticalSide & calculated,
                           double tolerance);

    /// \brief Asserts that the given optical type contains the expected values.
    /// \param correct The correct optical type.
    /// \param calculated The calculated optical type.
    /// \param tolerance The tolerance for comparing the values.
    void assertOpticalRange(const MaterialsLibrary::OpticalType & correct,
                           const std::optional<MaterialsLibrary::OpticalType> & calculated,
                           double tolerance);

    void assertInfraredSection(const MaterialsLibrary::Infrared & correct,
                               const MaterialsLibrary::Infrared & calculated,
                               double tolerance);


    //////////////////////////////////////////////////////////////////////////////////////////
    //// Helper functions to fill the data
    //////////////////////////////////////////////////////////////////////////////////////////

    /// \brief Inserts a TableValue node into the given mock node.
    /// \param node Node at which the TableValue node will be inserted.
    /// \param x Value of the x-coordinate.
    /// \param y Value of the y-coordinate.
    void addTableValueNode(Helper::MockNode & node, std::string_view childName, std::string_view x, std::string_view y);

    /// \brief Fills the given parent node with the table values.
    /// \param parentNode The parent node to be filled with the table values.
    /// \param parentName The name of the parent node.
    /// \param tableValueNodeName The name of the table value node.
    /// \param values The values to be inserted into the parent node.
    void fillNodesWithTableValues(MockNode & parentNode,
                                  const std::string & parentName,
                                  const std::string & tableValueNodeName,
                                  const std::vector<std::pair<std::string, std::string>> & values);

    /// \brief Fills the given parent node with the optical properties.
    /// \param parentNode The parent node to be filled with the optical properties.
    /// \param sideName The name of the side node.
    /// \param transmittanceValue The value of the transmittance that will be filled into the node
    /// \param reflectanceValue The value of the reflectance that will be filled into the node
    void fillOpticalPropertiesNode(Helper::MockNode & parentNode,
                                   std::string_view sideName,
                                   std::string_view transmittanceValue,
                                   std::string_view reflectanceValue);

    using TestProperty = std::pair<std::string, std::pair<std::string_view, std::string_view>>;
    using TestPropertyList = std::vector<TestProperty>;

    /// \brief Fills the specified side of a node with a list of optical properties.
    /// \param node The node to be filled with optical properties.
    /// \param sideName The name of the side (e.g., "Direct", "Diffuse").
    /// \param properties A list of properties (name and value pairs) to be applied to the node.
    void fillOpticalSide(Helper::MockNode & node, std::string_view sideName, const TestPropertyList & properties);

    using RangeProperty = std::pair<std::string, TestPropertyList>;

    /// \brief Fills a node with a range of optical properties.
    /// \param node The node to be filled.
    /// \param rangeName The name of the optical range (e.g., "Solar", "Visible").
    /// \param properties A vector of range properties, each with a side name and a list of properties.
    void fillOpticalRange(Helper::MockNode & node,
                          std::string_view rangeName,
                          const std::vector<RangeProperty> & properties);

    /// \brief Creates an infrared section within the specified node.
    /// \param node The node where the infrared section will be created.
    /// \param transmittance The transmittance value for the infrared section.
    /// \param emissivityFront The emissivity value for the front side of the infrared section.
    /// \param emissivityBack The emissivity value for the back side of the infrared section.
    void fillInfraredSection(MockNode & node,
                             std::optional<std::string_view> transmittance,
                             std::optional<std::string_view> emissivityFront,
                             std::optional<std::string_view> emissivityBack);

    /// \brief Creates one example of solid material for in memory testing
    void fillSolidMaterialTest1(MockNode & node);
}   // namespace Helper
