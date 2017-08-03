#include "gtest/gtest.h"

extern "C"
{
	int walled_garden_format_cleanup(char *entries, int entries_len);
}

class MantisTest : public ::testing::Test
{
protected:
	virtual	void SetUp() {

	}

	virtual void TearDown() {

	}
};

TEST_F(MantisTest, ShouldSuccessIfWalledGardenFormatCleanup)
{
	char entries[256] = {0};
	memcpy(entries, "captive,portal,techevo,test", strlen("captive,portal,techevo,test"));
	walled_garden_format_cleanup(entries, 256);
}