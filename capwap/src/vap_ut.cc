#include "gtest/gtest.h"

#include "json.h"
#include "json_object.h"
#include "cwAcAccount.h"
#include "cwAcThread.h"
#include "capwap_defs.h"

extern "C"
{
	void wlan_vap_handler(cwAccountCtx_t * account_ctx, struct json_object *req, void *_s);
}

class VapTest : public ::testing::Test
{
protected:
	virtual	void SetUp() {
		cwConf.thread_num = 1;
		cwConf.vap_per_apnetwork = 100;
		initAccountContext();

		req = NULL;
		rsp = json_object_new_object();

		memset(&thread, 0, sizeof(thread));
		current = &thread;
	}

	virtual void TearDown() {
		clearAccountContext();
		if (req) json_object_put(req);
		if (rsp) json_object_put(rsp);
	}

private:
	virtual void initAccountContext() {
		init_account_ctx();

		memset(&accountInfo, 0, sizeof(accountInfo));
		accountInfo.oid = 401;
		context = add_account_ctx(&accountInfo);
	}

	virtual void clearAccountContext()
	{
		int result = cwAcDeleteAccount(&accountInfo);
		ASSERT_EQ(0, result);
	}

public:
	cwAccountCtx_t *context;
	cwAcThread_t thread;
	struct json_object *req;
	struct json_object *rsp;

private:
	cw_account_info_t accountInfo;
};

TEST_F(VapTest, ShouldReturnSuccessIfPutConfig) {
	req = json_object_from_file("json/vap_ut.json");
	wlan_vap_handler(context, req, rsp);
	ASSERT_EQ(0, json_object_get_int(json_object_object_get(rsp, "code")));
	ASSERT_STREQ("ok", json_object_get_string(json_object_object_get(rsp, "message")));

	cw_rb_node_t *node;
	cwWlanInfo_t *wlan;
	rb_for_each(node, context->cw_wl_tree) {
		wlan = rb_entry(node, cwWlanInfo_t, rb_node);
		if (wlan && !strcmp(wlan->ssid, "zqq-ssid-5")) {
			break;
		}
	}
	ASSERT_EQ(1046, wlan->oid);
	ASSERT_EQ(context, wlan->account_ctx);
	ASSERT_STREQ("zqq-ssid-5", (char*)wlan->name);
	ASSERT_STREQ("zqq-ssid-5", wlan->ssid);
	ASSERT_EQ(CW_MAC_TYPE_ENUM_LOCAL, wlan->mac_type);
	ASSERT_EQ(CW_TUNNEL_TYPE_ENUM_8023, wlan->tunnel_type);
	ASSERT_EQ(CW_WL_SEC_NONE, wlan->security);
	ASSERT_EQ(0, wlan->cpauth);
	ASSERT_EQ(0, wlan->auth);
	ASSERT_EQ(1, wlan->okc);
	ASSERT_EQ(0, wlan->encrypt);
	ASSERT_EQ(1, wlan->intra_privacy);
	ASSERT_EQ(1, wlan->schedule);
	ASSERT_STREQ("1:ALL|2:ALL|3:ALL|4:ALL|5:ALL|6:ALL|0:ALL", wlan->scheduleTime);
	ASSERT_EQ(APPLY_TO_ALL_TAG, wlan->tagBits);
	ASSERT_EQ(3, wlan->bandAvailability);
}

TEST_F(VapTest, ShouldReturnSuccessIfGetConfig) {
	req = json_object_new_object();
	json_object_object_add(req, "method", json_object_new_string("get"));
	wlan_vap_handler(context, req, rsp);
	ASSERT_EQ(0, json_object_get_int(json_object_object_get(rsp, "code")));
	ASSERT_STREQ("ok", json_object_get_string(json_object_object_get(rsp, "message")));
}