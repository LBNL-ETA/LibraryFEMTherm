#include <filesystem>
#include <gtest/gtest.h>

#include <BCSteadyState/DB.hxx>

TEST(TestSteadyStateBC, Deserialization)
{
    SCOPED_TRACE("Begin Test: Load ID50_CS_single.thmz");
    std::filesystem::path product_path(TEST_DATA_DIR);
    product_path /= "products";
    product_path /= "ID50_CS_single.thmz";

    BCSteadyStateLibrary::DB lib;
    lib.loadFromZipFile(product_path.string());

    auto names{lib.getNames()};

    EXPECT_EQ(3u, lib.getNames().size());
    EXPECT_EQ("Adiabatic", names[0]);
    EXPECT_EQ("ShadeInE", names[1]);
    EXPECT_EQ("ShadeOutE", names[2]);
}
