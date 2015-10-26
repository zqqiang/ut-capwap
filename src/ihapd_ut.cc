#include "gtest/gtest.h"

#include "hostapd.h"
#include "ieee802_11.h"
#include "cwAC.h"

extern "C"
{

}

class IhapdTest : public ::testing::Test
{
protected:
	virtual void SetUp() {
		InitData();
		InitConf();
		InitMgmt();
		InitMsg();
		InitHapd();
	}

	virtual void TearDown()	{
		memset(&msg, 0, sizeof(msg));
		memset(&hapd, 0, sizeof(hapd));
	}

protected:
	void InitData() {
		u8 defaultDest[6] = {0x08, 0x5b, 0x0e, 0xe9, 0xe6, 0x76};
		memcpy(dest, defaultDest, sizeof(defaultDest));

		u8 defaultSource[6] = {0xf8, 0xcf, 0xc5, 0x7d, 0x4f, 0xd1};
		memcpy(source, defaultSource, sizeof(defaultSource));
	}

	void InitHapd() {
		memset(&hapd, 0, sizeof(hapd));

		memset(&interfaces, 0, sizeof(interfaces));
		interfaces.log_enable = 0;

		memset(&iface, 0, sizeof(iface));
		iface.interfaces = &interfaces;

		hapd.iface = &iface;

		memcpy(hapd.own_addr, dest, sizeof(dest));
		hapd.splitMac = 1;
		hapd.conf = &config;
	}

	void InitMgmt() {
		memset(&mgmt, 0, sizeof(mgmt));
		memcpy(mgmt.bssid, dest, sizeof(dest));
		memcpy(mgmt.da, dest, sizeof(dest));
		memcpy(mgmt.sa, source, sizeof(source));
	}

	void InitMsg() {
		memset(&msg, 0, sizeof(msg));
		memcpy(msg.buf, &mgmt, sizeof(mgmt));
		msg.data = msg.buf;
		msg.len = sizeof(mgmt);
	}

	void InitConf() {
		memset(&config, 0, sizeof(config));
		config.max_num_sta = 100;
	}

protected:
	cw_msg_t msg;
	struct hostapd_data hapd;

private:
	struct hostapd_iface iface;
	struct hapd_interfaces interfaces;
	struct ieee80211_mgmt mgmt;
	struct hostapd_bss_config config;
	u8 dest[6];
	u8 source[6];
};

TEST_F(IhapdTest, ShouldAddUnknowStaSuccess)
{
	cw_hapd_80211_input(&hapd, msg.data, msg.len);
}