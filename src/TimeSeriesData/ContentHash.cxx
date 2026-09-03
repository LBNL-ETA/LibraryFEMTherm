#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <format>
#include <optional>
#include <string>

#include "ContentHash.hxx"
#include "Tags.hxx"

namespace TimeSeriesLibrary
{
    namespace
    {
        constexpr uint64_t offsetBasis{14695981039346656037ULL};
        constexpr uint64_t secondSeed{0x9E3779B97F4A7C15ULL};

        //! FNV-1a, 64 bit. Deterministic and platform-independent for identical input bytes;
        //! doubles are hashed by their IEEE-754 bit patterns so no formatting is involved.
        class Fnv1a
        {
        public:
            explicit Fnv1a(uint64_t seed) : m_State{seed}
            {}

            void consume(uint64_t bits)
            {
                for(size_t byteIndex = 0U; byteIndex < sizeof(bits); ++byteIndex)
                {
                    const auto byte{static_cast<uint8_t>(bits >> (8U * byteIndex))};
                    m_State ^= byte;
                    m_State *= prime;
                }
            }

            void consume(const std::string & text)
            {
                for(const char character : text)
                {
                    m_State ^= static_cast<uint8_t>(character);
                    m_State *= prime;
                }
            }

            [[nodiscard]] uint64_t value() const
            {
                return m_State;
            }

        private:
            static constexpr uint64_t prime{1099511628211ULL};
            uint64_t m_State;
        };

        void consumeSeries(Fnv1a & hasher, const std::vector<Series> & seriesList)
        {
            // Series order must not affect the hash: hash each series separately into a
            // per-series digest, then combine order-independently via sorted digests.
            std::vector<uint64_t> digests;
            digests.reserve(seriesList.size());

            for(const auto & series : seriesList)
            {
                Fnv1a seriesHasher{offsetBasis};
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
        Fnv1a first{offsetBasis};
        consumeAxis(first, data.axis);
        consumeSeries(first, data.series);

        Fnv1a second{secondSeed};
        consumeAxis(second, data.axis);
        consumeSeries(second, data.series);

        const uint64_t high{first.value()};
        const uint64_t low{second.value()};

        return std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
                           static_cast<uint32_t>(high >> 32U),
                           static_cast<uint16_t>(high >> 16U),
                           static_cast<uint16_t>(high),
                           static_cast<uint16_t>(low >> 48U),
                           low & 0xFFFFFFFFFFFFULL);
    }

    std::uint64_t datasetFingerprint(const TimeSeriesData & data)
    {
        Fnv1a hasher{offsetBasis};
        consumeMetadata(hasher, data);
        consumeAxis(hasher, data.axis);
        consumeSeries(hasher, data.series);
        return hasher.value();
    }
}   // namespace TimeSeriesLibrary
