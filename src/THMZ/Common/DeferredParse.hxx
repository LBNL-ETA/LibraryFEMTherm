#pragma once

#include <future>
#include <optional>

namespace ThermFile
{
    //! Handle for a heavyweight archive section whose parsing runs on a worker thread.
    //! Parsing starts when the handle is created (see the deferred* factories on the
    //! section DBs); take() blocks only when the parse has not finished yet. Single
    //! consumer: take() moves the section out, after which the handle is empty.
    template<typename Section>
    class DeferredParse
    {
    public:
        DeferredParse() = default;

        explicit DeferredParse(std::future<std::optional<Section>> parseFuture) :
            future_{std::move(parseFuture)}
        {}

        [[nodiscard]] bool valid() const
        {
            return future_.valid();
        }

        [[nodiscard]] std::optional<Section> take()
        {
            if(!future_.valid())
            {
                return std::nullopt;
            }
            return future_.get();
        }

    private:
        std::future<std::optional<Section>> future_;
    };
}   // namespace ThermFile
