#include "gtest/gtest.h"
#include "mockcpp/mokc.h"

#include "hostapd.h"
#include "ieee802_11.h"
#include "cwAC.h"
#include "wtphtree.h"

extern "C"
{
	int cwHapdInit(cwAcThread_t *self);
	int cw_sta_load_chk(account_t account, uint32_t wtpAddr, uint32_t wtpPort,
	                    uint8_t rId, uint8_t wId, uint8_t *sta_mac, uint8_t mesh_sta, int *band_5G);
	wtp_hash_t* cwAcAddWtpHashEntry(cwAccountCtx_t *account_ctx, capwap_wtp_t *wtp, int add_to_hash);
}

class IhapdTest : public ::testing::Test
{
protected:
	virtual void SetUp() {
		InitData();
		InitConf();
		InitMgmt();
		InitMsg();
		InitHapdConfig();
		InitHapd();
		InitCallback();
		InitWtpInfo();
		InitStaInfo();
		InitAccountContext();
		InitWtpHash();
		InitSession();
		InitWtpProfileInfo();
		InitWlanInfo();
	}

	virtual void TearDown()	{
		memset(&msg, 0, sizeof(msg));
		memset(&hapd, 0, sizeof(hapd));

		GlobalMockObject::verify();
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

		hapd.wtpAddr = 0xffffffff;
		hapd.wtpPort = 8888;

		hapd.iconf = &hapdConfig;
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

	void InitCallback() {
		cw_hapd_sta_chk = cw_sta_load_chk;
	}

	void InitSession() {
		memset(&session, 0, sizeof(session));
		session.wtpcfg = &wtpInfo;
		session.wtp_hash_entry = wtpHash;
		session.account_ctx = &accountContext;
	}

	void InitWtpInfo() {
		memset(&wtpInfo, 0, sizeof(wtpInfo));
		wtpInfo.wtpprof_cfg = &wtpProfileInfo;
		wtpInfo.wlan_cfg[0][0] = &wlanInfo;
	}

	void InitWtpHash() {
		wtp_hash_tree_init();
		memset(&wtp, 0, sizeof(wtp));
		wtpHash = cwAcAddWtpHashEntry(&accountContext, &wtp, 1);
		wtpHash->ip = 0xffffffff;
		wtpHash->ctrl_port = 8888;
		add_ip_ctrl_port_hash_entry(wtpHash);
		wtpHash->wtpInfo = &wtpInfo;
		wtpHash->wtpInfo->wtp_hash_entry = wtpHash;
		wtpHash->wtpInfo->ws = &session;
	}

	void InitAccountContext() {
		memset(&accountContext, 0, sizeof(accountContext));
		accountContext.cw_sta_tree = RB_ROOT(ETH_ALEN, 0, 0);
		staInfo.rb_key1 = staInfo.macAddr;
		staInfo.rb_key2 = NULL;
		staInfo.rb_key3 = NULL;
		cw_rb_insert(&(accountContext.cw_sta_tree), &staInfo);
	}

	void InitWtpProfileInfo() {
		memset(&wtpProfileInfo, 0, sizeof(wtpProfileInfo));
	}

	void InitWlanInfo() {
		memset(&wlanInfo, 0, sizeof(wlanInfo));
	}

	void InitStaInfo() {
		memset(&staInfo, 0, sizeof(staInfo));
		memcpy(staInfo.macAddr, source, sizeof(source));
	}

	void InitHapdConfig() {
		memset(&hapdConfig, 0, sizeof(hapdConfig));
	}

protected:
	cw_msg_t msg;
	struct hostapd_data hapd;

private:
	struct hostapd_iface iface;
	struct hapd_interfaces interfaces;
	struct ieee80211_mgmt mgmt;
	struct hostapd_bss_config config;
	cwAccountCtx_t accountContext;
	capwap_wtp_t wtp;
	wtp_hash_t* wtpHash;
	cwWtpSession_t session;
	cwWtpInfo_t wtpInfo;
	cwWtpprofInfo_t wtpProfileInfo;
	cwWlanInfo_t wlanInfo;
	cwStaInfo_t staInfo;
	struct hostapd_config hapdConfig;
	u8 dest[6];
	u8 source[6];
};

TEST_F(IhapdTest, ShouldAddUnknowStaSuccess)
{
	MOCKER(sendto).expects(once());

	cw_hapd_80211_input(&hapd, msg.data, msg.len);
}