#pragma once

namespace ThermFile
{
    struct ThermModel;
}   // namespace ThermFile

namespace Helper
{
    struct MockNode;

    void
      expect_near(const ThermFile::ThermModel & expected, const ThermFile::ThermModel & result, double tolerance);

}   // namespace Helper