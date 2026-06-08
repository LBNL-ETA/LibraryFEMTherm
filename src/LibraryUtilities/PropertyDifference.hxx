#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "LibraryUtilities/Common.hxx"

namespace LibraryCommon
{
    //! A single field value, kept raw so consumers (THERM UI, Python, ...) decide how to
    //! present it: monostate = absent ("N/A"), double = numeric, string = label.
    using PropertyValue = std::variant<std::monostate, double, std::string>;

    //! One physical difference between two records: a dash-stacked field path and its raw value
    //! on each side. No display formatting lives here - the caller builds any message it needs.
    struct PropertyDifference
    {
        std::string name;
        PropertyValue lhs;
        PropertyValue rhs;
    };

    //! ---- fieldEqual: how two values of a given field type compare ----
    //! Doubles use a tolerance; everything else falls back to operator==. Module-specific types
    //! reuse the generic operator== overload below.
    inline bool fieldEqual(const double lhs, const double rhs, const double tolerance)
    {
        return isEqual(lhs, rhs, tolerance);
    }

    inline bool fieldEqual(const std::optional<double> & lhs, const std::optional<double> & rhs, const double tolerance)
    {
        return isEqual(lhs, rhs, tolerance);
    }

    template<typename ValueType>
    bool fieldEqual(const ValueType & lhs, const ValueType & rhs, const double /*tolerance*/)
    {
        return lhs == rhs;
    }

    //! ---- toValue: raw value as a PropertyValue ----
    //! Field types that are neither numeric, string, nor bool need a module-specific toValue
    //! overload (found by ADL); there is intentionally no catch-all, so an un-handled field type
    //! is a compile error rather than a silent opaque row.
    inline PropertyValue toValue(const double value)
    {
        return value;
    }

    inline PropertyValue toValue(const std::optional<double> & value)
    {
        if(value.has_value())
        {
            return value.value();
        }
        return std::monostate{};
    }

    inline PropertyValue toValue(const std::optional<std::string> & value)
    {
        if(value.has_value() && !value->empty())
        {
            return value.value();
        }
        return std::monostate{};
    }

    inline PropertyValue toValue(const bool value)
    {
        return std::string{value ? "true" : "false"};
    }

    inline PropertyValue toValue(const std::string & value)
    {
        return value;
    }

    //! The real `visitFields` overloads live in each library module's namespace
    //! (MaterialsLibrary, BCSteadyStateLibrary, GasesLibrary) and are more specialized than
    //! this one. This declaration exists only to anchor name lookup so the collectors below can
    //! call the module overloads via ADL; it is intentionally never defined.
    template<typename Record, typename Visitor>
    void visitFields(const Record & lhs, const Record & rhs, Visitor & visitor);

    //! ---------------------------------------------------------------------------------------
    //! Generic recursive collectors.
    //!
    //! Each struct declares ONE `visitFields(a, b, visitor)` that calls, for each member:
    //!   visitor(name, a.x, b.x [, tol])     - a scalar leaf
    //!   visitor.nested(name, a.sub, b.sub)  - an always-present sub-record (recurses)
    //!   visitor.optional(name, a.opt, b.opt)- an optional sub-record (presence + recurse)
    //!
    //! Both `operator==` and `physicalDifferences` for that struct are then written by handing
    //! one of these collectors to `visitFields`. nested/optional recurse via `visitFields`
    //! (found by ADL on the member type), so the single field list drives equality AND the diff
    //! - they cannot drift. The DifferenceCollector stacks the path ("Solar-Direct-Front-...")
    //! and, when one side of an optional is absent, enumerates the present side against N/A.
    //! ---------------------------------------------------------------------------------------
    struct EqualityCollector
    {
        bool equal{true};

        template<typename ValueType>
        void operator()(const std::string_view /*name*/, const ValueType & lhs, const ValueType & rhs, const double tolerance = 1e-6)
        {
            if(!fieldEqual(lhs, rhs, tolerance))
            {
                equal = false;
            }
        }

        template<typename RecordType>
        void nested(const std::string_view /*name*/, const RecordType & lhs, const RecordType & rhs)
        {
            visitFields(lhs, rhs, *this);
        }

        template<typename RecordType>
        void optional(const std::string_view /*name*/, const std::optional<RecordType> & lhs, const std::optional<RecordType> & rhs)
        {
            if(lhs.has_value() != rhs.has_value())
            {
                equal = false;
                return;
            }
            if(lhs.has_value())
            {
                visitFields(lhs.value(), rhs.value(), *this);
            }
        }
    };

    struct DifferenceCollector
    {
        std::vector<PropertyDifference> diffs;
        std::string prefix{};
        int absentSide{0};   // 0 = compare; -1 = lhs absent (enumerate, lhs N/A); +1 = rhs absent

        std::string pathFor(const std::string_view name) const
        {
            if(name.empty())
            {
                return prefix;
            }
            if(prefix.empty())
            {
                return std::string{name};
            }
            return prefix + "-" + std::string{name};
        }

        template<typename ValueType>
        void operator()(const std::string_view name, const ValueType & lhs, const ValueType & rhs, const double tolerance = 1e-6)
        {
            if(absentSide < 0)
            {
                diffs.push_back({pathFor(name), std::monostate{}, toValue(rhs)});
            }
            else if(absentSide > 0)
            {
                diffs.push_back({pathFor(name), toValue(lhs), std::monostate{}});
            }
            else if(!fieldEqual(lhs, rhs, tolerance))
            {
                diffs.push_back({pathFor(name), toValue(lhs), toValue(rhs)});
            }
        }

        template<typename RecordType>
        void nested(const std::string_view name, const RecordType & lhs, const RecordType & rhs)
        {
            DifferenceCollector child{{}, pathFor(name), absentSide};
            visitFields(lhs, rhs, child);
            diffs.insert(diffs.end(), child.diffs.begin(), child.diffs.end());
        }

        template<typename RecordType>
        void optional(const std::string_view name, const std::optional<RecordType> & lhs, const std::optional<RecordType> & rhs)
        {
            // Already enumerating a present side: descend into whichever side we are walking.
            if(absentSide < 0)
            {
                if(rhs.has_value())
                {
                    nested(name, rhs.value(), rhs.value());
                }
                return;
            }
            if(absentSide > 0)
            {
                if(lhs.has_value())
                {
                    nested(name, lhs.value(), lhs.value());
                }
                return;
            }
            if(lhs.has_value() && rhs.has_value())
            {
                nested(name, lhs.value(), rhs.value());
            }
            else if(lhs.has_value())
            {
                DifferenceCollector child{{}, pathFor(name), 1};
                visitFields(lhs.value(), lhs.value(), child);
                diffs.insert(diffs.end(), child.diffs.begin(), child.diffs.end());
            }
            else if(rhs.has_value())
            {
                DifferenceCollector child{{}, pathFor(name), -1};
                visitFields(rhs.value(), rhs.value(), child);
                diffs.insert(diffs.end(), child.diffs.begin(), child.diffs.end());
            }
        }
    };

    //! Presence marker for cases handled outside visitFields (a variant alternative, a vector
    //! component): "set" when present, absent otherwise.
    inline PropertyValue presenceValue(const bool present)
    {
        if(present)
        {
            return std::string{"set"};
        }
        return std::monostate{};
    }
}   // namespace LibraryCommon
