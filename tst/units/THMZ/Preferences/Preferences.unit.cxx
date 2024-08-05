#include "gtest/gtest.h"

#include <variant>

#include "THMZ/Preferences/Serializers.hxx"

#include "PreferencesHelper.hxx"
#include "MockNodeAdapter.hxx"

class TestPreferences : public testing::Test
{};

TEST_F(TestPreferences, UnderlayDeserializer)
{
    auto elementNode(
      Helper::generateUnderlayNode({"UnderlayName", "true", "SI", {"1.0", "2.0"}, "None", "1.0", "1.0"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Underlay underlay;
    adapter >> underlay;

    Helper::expect_near(
      ThermFile::Underlay{
        "UnderlayName", true, ThermFile::UnitSystem::SI, {1.0, 2.0}, ThermFile::UnderlayType::None, 1.0, 1.0},
      underlay,
      1e-6);
}

TEST_F(TestPreferences, UnderlaySerializer)
{
    ThermFile::Underlay underlay{
      "UnderlayName", false, ThermFile::UnitSystem::IP, {1.0, 2.0}, ThermFile::UnderlayType::None, 1.0, 1.0};

    Helper::MockNode elementNode("Underlay");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << underlay;

    auto correctNodes = Helper::generateUnderlayNode({"UnderlayName", "false", "IP", {"1", "2"}, "None", "1", "1"});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestPreferences, ModelPreferencesDeserializer)
{
    auto elementNode =
      Helper::generateSettingsNode({{"1.0", "2.0"}, "3.0", "BtuPerHr_Ft_Fahrenheit", "Y-projected", "false"});
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Settings modelPreferences;
    adapter >> modelPreferences;

    Helper::expect_near(ThermFile::Settings{{1.0, 2.0},
                                            3.0,
                                            ThermFile::ConductivityUnits::BtuPerHr_Ft_Fahrenheit,
                                            ThermFile::UValueBasisType::YProjected,
                                            false},
                        modelPreferences,
                        1e-6);
}

TEST_F(TestPreferences, ModelPreferencesSerializer)
{
    ThermFile::Settings modelPreferences{{1.0, 2.0},
                                         3.0,
                                         ThermFile::ConductivityUnits::BtuPerHr_Ft_Fahrenheit,
                                         ThermFile::UValueBasisType::YProjected,
                                         false};

    Helper::MockNode elementNode("Settings");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << modelPreferences;

    auto correctNodes = Helper::generateSettingsNode({{"1", "2"}, "3", "Btu/hr-ft-F", "Y-projected", "false"});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestPreferences, PreferencesDeserializer)
{
    auto elementNode = Helper::generatePreferencesNode(
      {{"UnderlayName", "true", "SI", {"1.0", "2.0"}, "None", "1.0", "1.0"},
       {{"1.0", "2.0"}, "3.0", "BtuPerHr_Ft_Fahrenheit", "Y-projected", "false"},
       {"false", "false", "false", "false", "false", "false", "false", "false", "0", "0", "0", {"0", "0"}}});
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Preferences preferences;
    adapter >> preferences;

    Helper::expect_near(
      ThermFile::Preferences{
        ThermFile::Underlay{
          "UnderlayName", true, ThermFile::UnitSystem::SI, {1.0, 2.0}, ThermFile::UnderlayType::None, 1.0, 1.0},
        ThermFile::Settings{{1.0, 2.0},
                            3.0,
                            ThermFile::ConductivityUnits::BtuPerHr_Ft_Fahrenheit,
                            ThermFile::UValueBasisType::YProjected,
                            false}},
      preferences,
      1e-6);
}

TEST_F(TestPreferences, PreferencesSerializer)
{
    ThermFile::Preferences preferences{
      ThermFile::Underlay{
        "UnderlayName", false, ThermFile::UnitSystem::IP, {1.0, 2.0}, ThermFile::UnderlayType::None, 1.0, 1.0},
      ThermFile::Settings{{1.0, 2.0},
                          3.0,
                          ThermFile::ConductivityUnits::Btu_InchPerHour_FootSquared_Fahrenheit,
                          ThermFile::UValueBasisType::YProjected,
                          false},
      ThermFile::Snap{false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, {0.0, 0.0}}};

    Helper::MockNode elementNode("Preferences");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << preferences;

    auto correctNodes = Helper::generatePreferencesNode(
      {{"UnderlayName", "false", "IP", {"1", "2"}, "None", "1", "1"},
       {{"1", "2"}, "3", "Btu-in/hr-ft2-F", "Y-projected", "false"},
       {"false", "false", "false", "false", "false", "false", "false", "false", "0", "0", "0", {"0", "0"}}});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}