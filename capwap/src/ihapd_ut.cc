#include "gtest/gtest.h"
#include "mockcpp/mokc.h"

#include "hostapd.h"
#include "ieee802_11.h"
#include "cwAC.h"
#include "wtphtree.h"
#include "cwPktUtils.h"

extern "C"
{
	int cwHapdInit(cwAcThread_t *self);
	int cw_sta_load_chk(account_t account, uint32_t wtpAddr, uint32_t wtpPort,
	                    uint8_t rId, uint8_t wId, uint8_t *sta_mac, uint8_t mesh_sta, int *band_5G);
	wtp_hash_t* cwAcAddWtpHashEntry(cwAccountCtx_t *account_ctx, capwap_wtp_t *wtp, int add_to_hash);
	void cwHapdSendMsgToLocal(cwIpcMsg_t *pkt, int len);
}

class IhapdTest : public ::testing::Test
{
protected:
	virtual void SetUp() {
		InitData();
		InitConf();
		InitMgmt();
		InitMsg();
		InitSession();
		InitWtpInfo();
		InitWtpHash();
		InitHapdConfig();
		InitHapd();
		InitCallback();
		InitStaInfo();
		InitAccountContext();
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
		hapd.account = 401;

		memset(&interfaces, 0, sizeof(interfaces));
		interfaces.log_enable = 0;

		memset(&iface, 0, sizeof(iface));
		iface.interfaces = &interfaces;
		iface.current_mode = &hapdHwMode;
		iface.current_rates = NULL;
		iface.wtp_hash_entry = wtpHash;

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
		u8 pcap[] = {
			//IEEE 802.11 Association Request, Flags: ........
			0x00, 0x00, 0x3a, 0x01, 0x08, 0x5b, 0x0e, 0xe9, 0xe6, 0x76, 0xf8, 0xcf, 0xc5, 0x7d, 0x4f, 0xd1,
			0x08, 0x5b, 0x0e, 0xe9, 0xe6, 0x76, 0x70, 0xfd,
			//IEEE 802.11 wireless LAN management frame
			0x21, 0x84, 0x01, 0x00, 0x00, 0x0a, 0x7a, 0x71, 0x71, 0x2d, 0x73, 0x73, 0x69, 0x64, 0x2d, 0x31,
			0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24, 0x32, 0x04, 0x30, 0x48, 0x60, 0x6c,
			0x2d, 0x1a, 0x2c, 0x01, 0x03, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdd, 0x07, 0x00, 0x50,
			0xf2, 0x02, 0x00, 0x01, 0x00, 0x7f, 0x04, 0x00, 0x00, 0x0a, 0x02
		};
		memset(&msg, 0, sizeof(msg));
		memcpy(msg.buf, &pcap, sizeof(pcap));
		msg.data = msg.buf;
		msg.len = sizeof(pcap);
	}

	void InitConf() {
		memset(&config, 0, sizeof(config));
		config.max_num_sta = 100;
		memcpy(config.ssid.ssid, "zqq-ssid-1", strlen("zqq-ssid-1"));
		config.ssid.ssid_len = strlen("zqq-ssid-1");
		config.max_listen_interval = 1;
	}

	void InitCallback() {
		cw_hapd_message_sta = cwAc_log_sta;
		cw_hapd_add_cw_hdr = cw_add_cw_hdr;
		cw_hapd_sendto = cwAcWsTxPkt;
		cw_hapd_sta_chk = cw_sta_load_chk;
	}

	void InitSession() {
		memset(&session, 0, sizeof(session));
		session.wtpcfg = &wtpInfo;
		// session.wtp_hash_entry = wtpHash;
		session.account_ctx = &accountContext;
	}

	void InitWtpInfo() {
		memset(&wtpInfo, 0, sizeof(wtpInfo));
		wtpInfo.wtpprof_cfg = &wtpProfileInfo;
		wtpInfo.wlan_cfg[0][0] = &wlanInfo;
		wtpInfo.ws = &session;
	}

	void InitWtpHash() {
		wtp_hash_tree_init();
		memset(&wtp, 0, sizeof(wtp));
		wtpHash = cwAcAddWtpHashEntry(&accountContext, &wtp, 1);
		wtpHash->ip = 0xffffffff;
		wtpHash->ctrl_port = 8888;
		wtpHash->wtpInfo = &wtpInfo;
		add_ip_ctrl_port_hash_entry(wtpHash);
		
		session.wtp_hash_entry = wtpHash;
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

	void InitHapdHwMode() {
		memset(&hapdHwMode, 0, sizeof(hapdHwMode));
	}

protected:
	void buildMockMsg() {
		memset(&mockMsg, 0, sizeof(mockMsg));

		mockMsg.wtp_hash_entry = NULL;
		mockMsg.account = hapd.account;
		mockMsg.wtpAddr = hapd.wtpAddr;
		mockMsg.wtpPort = hapd.wtpPort;

		cwIpcStaInfo_t *s = (cwIpcStaInfo_t *)mockMsg.data;
		s->account = hapd.account;
		s->wtpAddr = hapd.wtpAddr;
		s->wtpPort = hapd.wtpPort;
		s->rId = hapd.conf->radioId;
		s->aId = 1;
		s->flags = 3;
		s->mesh_vap = 0;
		s->band = 3;

		memcpy(s->bssid, hapd.own_addr, ETH_ALEN);
		memcpy(s->macAddr, source, ETH_ALEN);
		s->cap = 33825;
		s->wId = hapd.conf->wlanId;
		s->rate_num = 12;

		u8 rates[] = {0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c};
		memcpy(s->rates, rates, 12);
		s->reason = 0;
		s->vName_len = strlen(hapd.conf->ssid.ssid);
		memcpy(s->vName, hapd.conf->ssid.ssid, s->vName_len);
		mockMsg.type = 256;
	}

protected:
	cw_msg_t msg;
	struct hostapd_data hapd;
	cwIpcMsg_t mockMsg;

public:
	wtp_hash_t* wtpHash;

private:
	struct hostapd_iface iface;
	struct hapd_interfaces interfaces;
	struct ieee80211_mgmt mgmt; // todo
	struct hostapd_bss_config config;
	cwAccountCtx_t accountContext;
	capwap_wtp_t wtp;
	cwWtpSession_t session;
	cwWtpInfo_t wtpInfo;
	cwWtpprofInfo_t wtpProfileInfo;
	cwWlanInfo_t wlanInfo;
	cwStaInfo_t staInfo;
	struct hostapd_config hapdConfig;
	struct hostapd_hw_modes hapdHwMode;
	u8 dest[6];
	u8 source[6];
};

#include <iostream>

struct PacketChecker
{
	PacketChecker(IhapdTest *self) : that(self) {}
	bool operator()(cwIpcMsg_t *packet) {
		if (packet->wtp_hash_entry != that->wtpHash) {
			std::cout << "packet->wtp_hash_entry != that->wtpHash" << std::endl;
			return false;
		}
		return true;
	}
	IhapdTest *that;
};

TEST_F(IhapdTest, ShouldAddUnknowStaSuccess)
{
	buildMockMsg();

	MOCKER(cwHapdSendMsgToLocal)
	.expects(once())
	.with(checkWith(PacketChecker(this)))
	.will(returnValue(0));

	cw_hapd_80211_input(&hapd, msg.data, msg.len);
}