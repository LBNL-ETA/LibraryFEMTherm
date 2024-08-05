#include "cma1.hxx"

#include <CMA/Tags.hxx>

std::string TestCMA::testDatabase()
{
    CMALibrary::Tags tags;
    std::string fileContent{"<" + tags.rootTag() + ">\n"};
    fileContent += testEnvironment();
    fileContent += testGlazingConductance();
    fileContent += testSpacerConductivity();
    fileContent += testSingleLayer();
    fileContent += testDoubleLayer();
    fileContent += "</" + tags.rootTag() + ">";
    return fileContent;
}

std::string TestCMA::testEnvironment()
{
    return "\t<Environment>\n"
           "\t\t<Tin>25.5</Tin>\n"
           "\t\t<Tout>-14.12</Tout>\n"
           "\t</Environment>\n";
}

std::string TestCMA::testGlazingConductance()
{
    return "\t<GlazingConductance>\n"
           "\t\t<Low>0.234191</Low>\n"
           "\t\t<High>7.32155</High>\n"
           "\t</GlazingConductance>\n";
}

std::string TestCMA::testSpacerConductivity()
{
    return "\t<SpacerConductivity>\n"
           "\t\t<Low>0.02</Low>\n"
           "\t\t<High>30</High>\n"
           "\t</SpacerConductivity>\n";
}

std::string TestCMA::testSingleLayer()
{
    return "\t<SingleLayer>\n"
           "\t\t<Thickness>0.004</Thickness>\n"
           "\t\t<Conductivity>1.004</Conductivity>\n"
           "\t\t<Options>\n"
           "\t\t\t<Glazing>Low</Glazing>\n"
           "\t\t\t<Spacer>Low</Spacer>\n"
           "\t\t\t<hcin>4.415185</hcin>\n"
           "\t\t\t<hcout>27</hcout>\n"
           "\t\t\t<EmissivityIn>0.027</EmissivityIn>\n"
           "\t\t\t<EmissivityOut>0.85</EmissivityOut>\n"
           "\t\t</Options>\n"
           "\t\t<Options>\n"
           "\t\t\t<Glazing>Low</Glazing>\n"
           "\t\t\t<Spacer>High</Spacer>\n"
           "\t\t\t<hcin>5.415185</hcin>\n"
           "\t\t\t<hcout>28</hcout>\n"
           "\t\t\t<EmissivityIn>0.028</EmissivityIn>\n"
           "\t\t\t<EmissivityOut>0.86</EmissivityOut>\n"
           "\t\t</Options>\n"
           "\t\t<Options>\n"
           "\t\t\t<Glazing>High</Glazing>\n"
           "\t\t\t<Spacer>Low</Spacer>\n"
           "\t\t\t<hcin>6.529954</hcin>\n"
           "\t\t\t<hcout>29</hcout>\n"
           "\t\t\t<EmissivityIn>0.85</EmissivityIn>\n"
           "\t\t\t<EmissivityOut>0.87</EmissivityOut>\n"
           "\t\t</Options>\n"
           "\t\t<Options>\n"
           "\t\t\t<Glazing>High</Glazing>\n"
           "\t\t\t<Spacer>High</Spacer>\n"
           "\t\t\t<hcin>7.529954</hcin>\n"
           "\t\t\t<hcout>30</hcout>\n"
           "\t\t\t<EmissivityIn>0.86</EmissivityIn>\n"
           "\t\t\t<EmissivityOut>0.88</EmissivityOut>\n"
           "\t\t</Options>\n"
           "\t</SingleLayer>\n";
}

std::string TestCMA::testDoubleLayer()
{
    return "\t<DoubleLayer>\n"
           "\t\t<Thickness1>0.005</Thickness1>\n"
           "\t\t<Thickness2>0.002</Thickness2>\n"
           "\t\t<Conductivity1>1.005</Conductivity1>\n"
           "\t\t<Conductivity2>1.002</Conductivity2>\n"
           "\t\t<Options>\n"
           "\t\t\t<Glazing>Low</Glazing>\n"
           "\t\t\t<Spacer>Low</Spacer>\n"
           "\t\t\t<hcin>2.854252</hcin>\n"
           "\t\t\t<hcout>37</hcout>\n"
           "\t\t\t<EmissivityIn>0.85</EmissivityIn>\n"
           "\t\t\t<EmissivityOut>0.94</EmissivityOut>\n"
           "\t\t</Options>\n"
           "\t\t<Options>\n"
           "\t\t\t<Glazing>Low</Glazing>\n"
           "\t\t\t<Spacer>High</Spacer>\n"
           "\t\t\t<hcin>3.854252</hcin>\n"
           "\t\t\t<hcout>38</hcout>\n"
           "\t\t\t<EmissivityIn>0.86</EmissivityIn>\n"
           "\t\t\t<EmissivityOut>0.95</EmissivityOut>\n"
           "\t\t</Options>\n"
           "\t\t<Options>\n"
           "\t\t\t<Glazing>High</Glazing>\n"
           "\t\t\t<Spacer>Low</Spacer>\n"
           "\t\t\t<hcin>4.866122</hcin>\n"
           "\t\t\t<hcout>39</hcout>\n"
           "\t\t\t<EmissivityIn>0.87</EmissivityIn>\n"
           "\t\t\t<EmissivityOut>0.96</EmissivityOut>\n"
           "\t\t</Options>\n"
           "\t\t<Options>\n"
           "\t\t\t<Glazing>High</Glazing>\n"
           "\t\t\t<Spacer>High</Spacer>\n"
           "\t\t\t<hcin>5.866122</hcin>\n"
           "\t\t\t<hcout>40</hcout>\n"
           "\t\t\t<EmissivityIn>0.88</EmissivityIn>\n"
           "\t\t\t<EmissivityOut>0.97</EmissivityOut>\n"
           "\t\t</Options>\n"
           "\t</DoubleLayer>\n";
}
