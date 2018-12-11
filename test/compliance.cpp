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
        for (const auto& testSuite: testSuites)
        {
            const Json& document = testSuite["given"];
            const Json& testCases = testSuite["cases"];
            for (const auto& testCase: testCases)
            {
                auto expression = testCase["expression"]
                        .get_ref<const String&>();
                auto resultIt = testCase.find("result");
                if (resultIt != testCase.cend())
                {
                    testResult(expression, document, *resultIt);
                }
                auto errorIt = testCase.find("error");
                if (errorIt != testCase.cend())
                {
                    testError(expression, document, *errorIt);
                }
            }
        }
    }

    void testResult(const std::string& expression,
                    const Json& document,
                    const Json& expectedResult) const
    {
        Json result;
        try
        {
            result = search(expression, document);
        }
        catch(std::exception& exc)
        {
            FAIL("Exception: " + String(exc.what())
                 + "\nExpression: " + expression
                 + "\nExpected result: " + expectedResult.dump()
                 + "\nResult: " + result.dump());
        }

        if (result == expectedResult)
        {
            SUCCEED();
        }
        else
        {
            FAIL("Expression: " + expression
                 + "\nExpected result: " + expectedResult.dump()
                 + "\nResult: " + result.dump());
        }
    }

    void testError(const std::string& expression,
                   const Json& document,
                   const std::string& expectedError) const
    {
        if (expectedError == "syntax")
        {
            REQUIRE_THROWS_AS(search(expression, document),
                              SyntaxError);
        }
        else if (expectedError == "invalid-value")
        {
            REQUIRE_THROWS_AS(search(expression, document),
                              InvalidValue);
        }
        else if (expectedError == "invalid-type")
        {
            REQUIRE_THROWS_AS(search(expression, document),
                              InvalidFunctionArgumentType);
        }
        else if (expectedError == "invalid-arity")
        {
            REQUIRE_THROWS_AS(search(expression, document),
                              InvalidFunctionArgumentArity);
        }
        else if (expectedError == "unknown-function")
        {
            REQUIRE_THROWS_AS(search(expression, document),
                              UnknownFunction);
        }
    }

private:
    static const std::string s_relativePath;

    Json readTestSuites(const std::string& fileName) const
    {
        std::ifstream jsonFile;
        jsonFile.open(s_relativePath + "/" + fileName);
        REQUIRE(jsonFile.is_open());
        Json featureTest;
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

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Literals", "[literals]")
{
    executeFeatureTest("literal");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Basic expressions",
                 "[basic]")
{
    executeFeatureTest("basic");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Boolean expressions",
                 "[boolean]")
{
    executeFeatureTest("boolean");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Current node", "[current]")
{
    executeFeatureTest("current");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Escapes", "[escape]")
{
    executeFeatureTest("escape");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Filters", "[filters]")
{
    executeFeatureTest("filters");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Functions", "[functions]")
{
    executeFeatureTest("functions");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Index expressions",
                 "[indices]")
{
    executeFeatureTest("indices");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Multiselect expressions",
                 "[multiselect]")
{
    executeFeatureTest("multiselect");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Pipe expressions", "[pipe]")
{
    executeFeatureTest("pipe");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Slice expressions",
                 "[slice]")
{
    executeFeatureTest("slice");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Syntax", "[syntax]")
{
    executeFeatureTest("syntax");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Unicode", "[unicode]")
{
    executeFeatureTest("unicode");
}

TEST_CASE_METHOD(ComplianceTestFixture, "Compliance/Wildcard", "[wildcard]")
{
    executeFeatureTest("wildcard");
}
