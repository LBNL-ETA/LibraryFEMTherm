#include "Tags.hxx"

namespace BCSteadyStateLibrary
{
    std::string Tags::uuid() const
    {
        return uuid_;
    }

    std::string Tags::name() const
    {
        return name_;
    }

    std::string Tags::projectName() const
    {
        return projectName_;
    }

    std::string Tags::protectedTag() const
    {
        return protectedTag_;
    }

    std::string Tags::color() const
    {
        return color_;
    }

    std::string Tags::convection() const
    {
        return convection_;
    }

    std::string Tags::constantFlux() const
    {
        return constantFlux_;
    }

    std::string Tags::radiation() const
    {
        return radiation_;
    }

    std::string Tags::automaticEnclosure() const
    {
        return automaticEnclosure_;
    }

    std::string Tags::manualEnclosure() const
    {
        return manualEnclosure_;
    }

    std::string Tags::blackBodyRadiation() const
    {
        return blackBodyRadiation_;
    }

    std::string Tags::linearizedRadiation() const
    {
        return linearizedRadiation_;
    }

    std::string Tags::temperature() const
    {
        return temperature_;
    }

    std::string Tags::filmCoefficient() const
    {
        return filmCoefficient_;
    }

    std::string Tags::emissivity() const
    {
        return emissivity_;
    }

    std::string Tags::viewFactor() const
    {
        return viewFactor_;
    }

    std::string Tags::flux() const
    {
        return flux_;
    }

    std::string Tags::relativeHumidity() const
    {
        return relativeHumidity_;
    }

    std::string Tags::boundaryCondition() const
    {
        return boundaryCondition_;
    }

    std::string Tags::boundaryConditions() const
    {
        return boundaryConditions_;
    }

    std::string Tags::comprehensive() const
    {
        return comprehensive_;
    }

    std::string Tags::simplified() const
    {
        return simplified_;
    }

    std::string Tags::radiationSurface() const
    {
        return radiationSurface_;
    }

    std::string Tags::isDefault() const
    {
        return isDefault_;
    }

    std::string Tags::isIGUSurface() const
    {
        return isIGUSurface_;
    }

    std::string BoundaryConditionTagVisitor::operator()(const Comprehensive &) const
    {
        BCSteadyStateLibrary::Tags tag;
        return tag.comprehensive();
    }

    std::string BoundaryConditionTagVisitor::operator()(const Simplified &) const
    {
        BCSteadyStateLibrary::Tags tag;
        return tag.simplified();
    }

    std::string BoundaryConditionTagVisitor::operator()(const RadiationSurface &) const
    {
        BCSteadyStateLibrary::Tags tag;
        return tag.radiationSurface();
    }
}   // namespace BCSteadyState