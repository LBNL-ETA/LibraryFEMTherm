#include <format>

#include "StableUuid.hxx"

namespace LibraryCommon
{
    Fnv1a::Fnv1a(const std::uint64_t seed) : m_State{seed}
    {}

    void Fnv1a::consume(const std::uint64_t bits)
    {
        for(std::size_t byteIndex = 0U; byteIndex < sizeof(bits); ++byteIndex)
        {
            const auto byte{static_cast<std::uint8_t>(bits >> (8U * byteIndex))};
            m_State ^= byte;
            m_State *= prime;
        }
    }

    void Fnv1a::consume(const std::string_view text)
    {
        for(const char character : text)
        {
            m_State ^= static_cast<std::uint8_t>(character);
            m_State *= prime;
        }
    }

    std::uint64_t Fnv1a::value() const
    {
        return m_State;
    }

    std::string uuidFromDigests(const std::uint64_t high, const std::uint64_t low)
    {
        return std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
                           static_cast<std::uint32_t>(high >> 32U),
                           static_cast<std::uint16_t>(high >> 16U),
                           static_cast<std::uint16_t>(high),
                           static_cast<std::uint16_t>(low >> 48U),
                           low & 0xFFFFFFFFFFFFULL);
    }

    namespace
    {
        void consumeLengthPrefixed(Fnv1a & hasher, const std::string_view text)
        {
            hasher.consume(static_cast<std::uint64_t>(text.size()));
            hasher.consume(text);
        }

        std::uint64_t digestOf(const std::uint64_t seed,
                               const std::string_view scope,
                               const std::string_view name)
        {
            Fnv1a hasher{seed};
            consumeLengthPrefixed(hasher, scope);
            consumeLengthPrefixed(hasher, name);
            return hasher.value();
        }
    }   // namespace

    std::string stableUuid(const std::string_view scope, const std::string_view name)
    {
        return uuidFromDigests(digestOf(fnvOffsetBasis, scope, name), digestOf(fnvSecondSeed, scope, name));
    }
}   // namespace LibraryCommon
