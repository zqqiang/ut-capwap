#include "gtest/gtest.h"

#include "json.h"
#include "json_object.h"
#include "cwAcAccount.h"

extern "C"
{
	void wlan_apnetwork_handler(cwAccountCtx_t * account_ctx, struct json_object *req, void *_s);
}

class ApnetworkTest : public ::testing::Test
{
protected:
	virtual void SetUp() {
		cwConf.thread_num = 1;
		initAccountContext();

		req = NULL;
		rsp = json_object_new_object();
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
	struct json_object *req;
	struct json_object *rsp;

private:
	cw_account_info_t accountInfo;
};

TEST_F(ApnetworkTest, ShouldRetureErrorWhenWrongMethod) {
	req = json_object_new_object();
	json_object_object_add(req, "method", json_object_new_string("get"));
	wlan_apnetwork_handler(context, req, rsp);
	ASSERT_EQ(-3, json_object_get_int(json_object_object_get(rsp, "code")));
	ASSERT_STREQ("jsonrpc method not supported", json_object_get_string(json_object_object_get(rsp, "message")));
}

TEST_F(ApnetworkTest, ShouldRetureSuccessIfDelete) {
	req = json_tokener_parse("{\"method\":\"delete\", \"apNetworkOid\":\"401\"}");
	wlan_apnetwork_handler(context, req, rsp);
	ASSERT_EQ(0, json_object_get_int(json_object_object_get(rsp, "code")));
	ASSERT_STREQ("ok", json_object_get_string(json_object_object_get(rsp, "message")));
}