/****************************************************************************
**
** Author: Róbert Márki <gsmiko@gmail.com>
** Copyright (c) 2016 Róbert Márki
**
** This file is part of the jmespath.cpp project which is distributed under
** the MIT License (MIT).
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
**
****************************************************************************/
#define CATCH_CONFIG_MAIN
#include "fakeit.hpp"
#include "jmespath/jmespath.h"
#include <fstream>

using namespace jmespath;

class ComplianceTestFixture
{
public:

protected:
    void executeFeatureTest(const std::string& featureName) const
    {
        Json testSuites = readTestSuites(featureName + ".json");
        for (auto testSuite: testSuites)
        {
            Json document = testSuite["given"];
            Json testCases = testSuite["cases"];
            for (auto testCase: testCases)
            {
                String expression = testCase["expression"];
                Json expectedResult = testCase["result"];

                REQUIRE(search(expression, document) == expectedResult);
            }
        }
    }

private:
    static const std::string s_relativePath;

    Json readTestSuites(const std::string& fileName) const
    {
        Json featureTest;
        std::ifstream jsonFile;
        jsonFile.open(s_relativePath + "/" + fileName);
        REQUIRE(jsonFile.is_open());
        jsonFile >> featureTest;
        jsonFile.close();
        return featureTest;
    }
};
const std::string ComplianceTestFixture::s_relativePath{"compliance_tests"};

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Identifiers",
                 "[identifiers]")
{
    executeFeatureTest("identifiers");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Literals",
                 "[!hide][literals]")
{
    executeFeatureTest("literal");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Basic expressions",
                 "[!hide][basic]")
{
    executeFeatureTest("basic");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Boolean expressions",
                 "[!hide][boolean]")
{
    executeFeatureTest("boolean");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Current node",
                 "[!hide][current]")
{
    executeFeatureTest("current");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Filters",
                 "[!hide][filters]")
{
    executeFeatureTest("filters");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Functions",
                 "[!hide][functions]")
{
    executeFeatureTest("functions");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Index expressions",
                 "[!hide][indices]")
{
    executeFeatureTest("indices");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Multiselect expressions",
                 "[!hide][multiselect]")
{
    executeFeatureTest("multiselect");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Pipe expressions",
                 "[!hide][pipe]")
{
    executeFeatureTest("pipe");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Slice expressions",
                 "[!hide][slice]")
{
    executeFeatureTest("slice");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Syntax",
                 "[!hide][syntax]")
{
    executeFeatureTest("syntax");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Unicode",
                 "[!hide][unicode]")
{
    executeFeatureTest("unicode");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Wildcard",
                 "[!hide][wildcard]")
{
    executeFeatureTest("wildcard");
}
