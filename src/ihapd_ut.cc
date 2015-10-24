#include "gtest/gtest.h"

#include "hostapd.h"

extern "C"
{
	void *cw_hapd_get_hapd(struct hapd_interfaces *interfaces, u8 *buf, size_t len, u8 *bssid);
	void cw_hapd_80211_input(struct hostapd_data *hapd, u8 *buf, size_t len);
}

class IhapdTest : public ::testing::Test
{
protected:
	virtual void SetUp() {
		memset(&session, 0, sizeof(session));
		memset(&msg, 0, sizeof(msg));

		ifaces = session.radio[0].ifaces;
		hapd = NULL;
	}

	virtual void TearDown()	{

	}

protected:
	cwWtpSession_t session;
	cw_msg_t msg;

	void *ifaces;
	void *hapd;
};

TEST_F(IhapdTest, ShouldAddUnknowStaSuccess)
{
	hapd = cw_hapd_get_hapd((struct hapd_interfaces *)ifaces, msg.data, msg.len, NULL);
	cw_hapd_80211_input((struct hostapd_data *)hapd, msg.data, msg.len);
}