#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace LibraryCommon
{
    //! FNV-1a, 64 bit. Deterministic and platform-independent for identical input bytes:
    //! integers are consumed byte by byte from the low end, text byte by byte as stored, so
    //! nothing here depends on formatting, locale or endianness of the caller.
    //!
    //! Shared by the dataset content hash (TimeSeriesData/ContentHash) and the name-derived
    //! record UUIDs below. Both produce identifiers that already live in archives, so the
    //! algorithm is a contract: changing it re-identifies every record ever generated.
    class Fnv1a
    {
    public:
        explicit Fnv1a(std::uint64_t seed);

        void consume(std::uint64_t bits);
        void consume(std::string_view text);

        [[nodiscard]] std::uint64_t value() const;

    private:
        static constexpr std::uint64_t prime{1099511628211ULL};
        std::uint64_t m_State;
    };

    //! The standard FNV offset basis, and a second seed (the 64-bit golden ratio) so two
    //! independent digests of the same input can be combined into one 128-bit identifier.
    constexpr std::uint64_t fnvOffsetBasis{14695981039346656037ULL};
    constexpr std::uint64_t fnvSecondSeed{0x9E3779B97F4A7C15ULL};

    //! Two 64-bit digests laid out as a UUID-shaped string, 8-4-4-4-12 lower-case hex. Not an
    //! RFC 4122 UUID -- no version or variant bits are set -- and none of the readers require
    //! one: THERM and the libraries treat a UUID as an opaque string of this shape.
    [[nodiscard]] std::string uuidFromDigests(std::uint64_t high, std::uint64_t low);

    //! A UUID derived from a name, the same on every machine and every run.
    //!
    //! For records that are generated rather than drawn: a material, a boundary-condition
    //! record or a dataset built from the same description must land on the same UUID each
    //! time, so that regenerating a model never leaves THERM with two library entries for one
    //! thing. ``scope`` keeps kinds apart -- a material and a boundary record that happen to
    //! share a name must not share an identifier. Both strings are length-prefixed before
    //! hashing, so ("ab", "c") and ("a", "bc") differ.
    [[nodiscard]] std::string stableUuid(std::string_view scope, std::string_view name);
}   // namespace LibraryCommon
