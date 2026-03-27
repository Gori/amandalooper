#include <juce_core/juce_core.h>

//==============================================================================
// AMLP Test Runner
//
// Only runs tests in the "AMLP" category.
// Register tests with: class MyTest : public juce::UnitTest { MyTest() : UnitTest("Name", "AMLP") {} };
//==============================================================================

int main (int, char**)
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false); // Don't crash on failure, report instead

    // Only run AMLP-category tests
    runner.runTestsInCategory ("AMLP");

    int totalTests = 0;
    int totalPasses = 0;
    int totalFailures = 0;

    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        if (auto* result = runner.getResult (i))
        {
            totalTests++;
            totalPasses += result->passes;
            totalFailures += result->failures;

            if (result->failures > 0)
            {
                std::cerr << "FAILED: " << result->unitTestName << "\n";
                for (auto& msg : result->messages)
                    std::cerr << "  " << msg << "\n";
            }
        }
    }

    std::cout << "\n";
    std::cout << "=== AMLP TEST RESULTS ===\n";
    std::cout << "Tests run: " << totalTests << "\n";
    std::cout << "Passes:    " << totalPasses << "\n";
    std::cout << "Failures:  " << totalFailures << "\n";

    if (totalTests == 0)
    {
        std::cout << "(No AMLP tests registered yet)\n";
        std::cout << "=== OK (no tests) ===\n";
        return 0;
    }

    if (totalFailures > 0)
    {
        std::cout << "=== " << totalFailures << " TEST(S) FAILED ===\n";
        return 1;
    }

    std::cout << "=== ALL TESTS PASSED ===\n";
    return 0;
}
