#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <format>

#include "ContentHash.hxx"
#include "Tags.hxx"

namespace EnvironmentDataLibrary
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

        void consumeChannels(Fnv1a & hasher, const std::vector<Channel> & channels)
        {
            // Channel order must not affect the hash: hash each channel separately into a
            // per-channel digest, then combine order-independently via sorted digests.
            std::vector<uint64_t> digests;
            digests.reserve(channels.size());

            for(const auto & channel : channels)
            {
                Fnv1a channelHasher{offsetBasis};
                channelHasher.consume(channelRoleToString(channel.role));
                for(const double value : channel.values)
                {
                    channelHasher.consume(std::bit_cast<uint64_t>(value));
                }
                digests.push_back(channelHasher.value());
            }

            std::ranges::sort(digests);
            for(const uint64_t digest : digests)
            {
                hasher.consume(digest);
            }
        }
    }   // namespace

    std::string contentUuid(const EnvironmentData & data)
    {
        Fnv1a first{offsetBasis};
        consumeChannels(first, data.channels);

        Fnv1a second{secondSeed};
        consumeChannels(second, data.channels);

        const uint64_t high{first.value()};
        const uint64_t low{second.value()};

        return std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
                           static_cast<uint32_t>(high >> 32U),
                           static_cast<uint16_t>(high >> 16U),
                           static_cast<uint16_t>(high),
                           static_cast<uint16_t>(low >> 48U),
                           low & 0xFFFFFFFFFFFFULL);
    }
}   // namespace EnvironmentDataLibrary
