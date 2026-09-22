#include <algorithm>
#include <bit>
#include <cstdint>
#include <optional>
#include <string>

#include "LibraryUtilities/StableUuid.hxx"

#include "ContentHash.hxx"
#include "Tags.hxx"

namespace TimeSeriesLibrary
{
    namespace
    {
        using LibraryCommon::Fnv1a;
        using LibraryCommon::fnvOffsetBasis;
        using LibraryCommon::fnvSecondSeed;

        //! Doubles are hashed by their IEEE-754 bit patterns, so no formatting is involved.
        void consumeSeries(Fnv1a & hasher, const std::vector<Series> & seriesList)
        {
            // Series order must not affect the hash: hash each series separately into a
            // per-series digest, then combine order-independently via sorted digests.
            std::vector<uint64_t> digests;
            digests.reserve(seriesList.size());

            for(const auto & series : seriesList)
            {
                Fnv1a seriesHasher{fnvOffsetBasis};
                seriesHasher.consume(seriesRoleToString(series.role));
                for(const double value : series.values)
                {
                    seriesHasher.consume(std::bit_cast<uint64_t>(value));
                }
                digests.push_back(seriesHasher.value());
            }

            std::ranges::sort(digests);
            for(const uint64_t digest : digests)
            {
                hasher.consume(digest);
            }
        }

        //! The axis is content: the same values a month apart, or at a different step, are
        //! different data and must not converge to one dataset.
        void consumeAxis(Fnv1a & hasher, const TimeAxis & axis)
        {
            hasher.consume(static_cast<uint64_t>(axis.month));
            hasher.consume(static_cast<uint64_t>(axis.day));
            hasher.consume(static_cast<uint64_t>(axis.hour));
            hasher.consume(static_cast<uint64_t>(axis.minute));
            hasher.consume(std::bit_cast<uint64_t>(axis.stepSeconds));
        }

        //! Strings are length-prefixed so adjacent fields cannot alias ("ab"+"c" vs
        //! "a"+"bc"); optionals additionally consume a presence flag so an absent field
        //! differs from a present-but-empty one.
        void consumeText(Fnv1a & hasher, const std::string & text)
        {
            hasher.consume(static_cast<uint64_t>(text.size()));
            hasher.consume(text);
        }

        void consumeOptionalText(Fnv1a & hasher, const std::optional<std::string> & text)
        {
            hasher.consume(text.has_value() ? 1ULL : 0ULL);
            consumeText(hasher, text.value_or(std::string{}));
        }

        void consumeMetadata(Fnv1a & hasher, const TimeSeriesData & data)
        {
            consumeText(hasher, data.UUID);
            consumeText(hasher, data.Name);
            consumeOptionalText(hasher, data.ProjectName);
            hasher.consume(data.Protected ? 1ULL : 0ULL);
            consumeOptionalText(hasher, data.Description);
            consumeText(hasher, data.Color);
            consumeOptionalText(hasher, data.Source);
        }
    }   // namespace

    std::string contentUuid(const TimeSeriesData & data)
    {
        Fnv1a first{fnvOffsetBasis};
        consumeAxis(first, data.axis);
        consumeSeries(first, data.series);

        Fnv1a second{fnvSecondSeed};
        consumeAxis(second, data.axis);
        consumeSeries(second, data.series);

        return LibraryCommon::uuidFromDigests(first.value(), second.value());
    }

    std::uint64_t datasetFingerprint(const TimeSeriesData & data)
    {
        Fnv1a hasher{fnvOffsetBasis};
        consumeMetadata(hasher, data);
        consumeAxis(hasher, data.axis);
        consumeSeries(hasher, data.series);
        return hasher.value();
    }
}   // namespace TimeSeriesLibrary
