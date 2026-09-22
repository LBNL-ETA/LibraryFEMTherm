#include <gtest/gtest.h>

#include <cctype>
#include <string>

#include "LibraryUtilities/StableUuid.hxx"

namespace
{
    bool isUuidShaped(const std::string & text)
    {
        if(text.size() != 36U)
        {
            return false;
        }
        for(std::size_t index = 0U; index < text.size(); ++index)
        {
            const bool dashPosition{index == 8U || index == 13U || index == 18U || index == 23U};
            if(dashPosition ? text[index] != '-' : !std::isxdigit(static_cast<unsigned char>(text[index])))
            {
                return false;
            }
        }
        return true;
    }
}   // namespace

TEST(StableUuidTest, DigestsLayOutAsUuid)
{
    EXPECT_EQ(LibraryCommon::uuidFromDigests(0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL),
              "01234567-89ab-cdef-fedc-ba9876543210");
    EXPECT_EQ(LibraryCommon::uuidFromDigests(0U, 0U), "00000000-0000-0000-0000-000000000000");
}

TEST(StableUuidTest, SameInputSameUuid)
{
    const auto first{LibraryCommon::stableUuid("material", "Stucco")};
    const auto second{LibraryCommon::stableUuid("material", "Stucco")};
    EXPECT_EQ(first, second);
    EXPECT_TRUE(isUuidShaped(first));
}

TEST(StableUuidTest, NameAndScopeBothMatter)
{
    const auto stucco{LibraryCommon::stableUuid("material", "Stucco")};
    EXPECT_NE(LibraryCommon::stableUuid("material", "stucco"), stucco);
    EXPECT_NE(LibraryCommon::stableUuid("boundary", "Stucco"), stucco);
    EXPECT_NE(LibraryCommon::stableUuid("", "Stucco"), stucco);
}

TEST(StableUuidTest, LengthPrefixPreventsAliasing)
{
    EXPECT_NE(LibraryCommon::stableUuid("ab", "c"), LibraryCommon::stableUuid("a", "bc"));
    EXPECT_NE(LibraryCommon::stableUuid("abc", ""), LibraryCommon::stableUuid("", "abc"));
}

TEST(StableUuidTest, IsPinned)
{
    // Generated records carry these identifiers in archives: the value is a contract.
    EXPECT_EQ(LibraryCommon::stableUuid("material", "Stucco"), "10418ea1-f55a-0907-70e6-4309a45738d7");
}

TEST(StableUuidTest, HasherMatchesReferenceVectors)
{
    // FNV-1a 64-bit reference: the empty input is the offset basis, "a" is 0xaf63dc4c8601ec8c.
    LibraryCommon::Fnv1a empty{LibraryCommon::fnvOffsetBasis};
    EXPECT_EQ(empty.value(), LibraryCommon::fnvOffsetBasis);

    LibraryCommon::Fnv1a single{LibraryCommon::fnvOffsetBasis};
    single.consume(std::string_view{"a"});
    EXPECT_EQ(single.value(), 0xaf63dc4c8601ec8cULL);
}
