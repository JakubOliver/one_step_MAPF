#include <gtest/gtest.h>

TEST(SmokeTest, AlwaysPasses) {
    SUCCEED();
}

TEST(SmokeTest, AlwaysFails) {
    FAIL() << "Intentional failure used to verify the GoogleTest setup.";
}
