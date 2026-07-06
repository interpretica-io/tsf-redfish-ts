/** @file
 * @brief Redfish Group
 *
 * Read a Redfish service's security posture with tapi_redfish_audit()
 * and gate on it. The endpoint is the test's to supply (env
 * TSF_REDFISH_URL); with none configured the test skips. The
 * default-credentials check logs in to a real BMC, so it stays OFF
 * unless TSF_REDFISH_DEFCREDS=1 (authorized assessment only). The gate
 * fails on any finding at least HIGH (an http-no-tls service, or an
 * accepted default credential).
 *
 * Copyright (C) 2026 Interpretica Unipessoal Lda
 */

#define TE_TEST_NAME    "redfish/audit"

#include "te_config.h"
#include <stdlib.h>
#include "tapi_test.h"
#include "te_string.h"

#include "tapi_cybersec.h"
#include "tapi_redfish_audit.h"
#include "tsapi_redfish.h"

int
main(int argc, char **argv)
{
    tsapi_redfish_session sess = {0};
    tapi_redfish_audit_policy policy = tapi_redfish_default_audit_policy;
    tapi_cybersec_report report;
    te_string verdict = TE_STRING_INIT;
    bool report_ready = false;
    const char *url;

    TEST_START;

    url = getenv("TSF_REDFISH_URL");
    if (url == NULL || url[0] == '\0')
        TEST_SKIP("Set TSF_REDFISH_URL (e.g. https://bmc) to point at a service");

    if (getenv("TSF_REDFISH_DEFCREDS") != NULL)
        policy.attempt_default_creds = true;

    TEST_STEP("Open a session to the agent");
    CHECK_RC(tsapi_redfish_session_init(&sess, "pco_redfish_audit"));

    TEST_STEP("Read the Redfish posture of %s into a report", url);
    tapi_cybersec_report_init(&report);
    report_ready = true;
    CHECK_RC(tapi_redfish_audit(sess.pco, url, &policy, &report));
    tapi_cybersec_report_log(&report);

    TEST_STEP("The report is well-formed");
    if (tapi_cybersec_report_count(&report, TAPI_CYBERSEC_SEV_INFO) == 0)
        TEST_VERDICT("the Redfish audit produced no findings at all");

    TEST_STEP("Gate: fail on anything at least HIGH");
    if (tapi_cybersec_report_verdict(&report, TAPI_CYBERSEC_SEV_HIGH, &verdict))
        TEST_VERDICT("%s", verdict.ptr);

    TEST_SUCCESS;

cleanup:
    te_string_free(&verdict);
    if (report_ready)
        tapi_cybersec_report_free(&report);
    tsapi_redfish_session_fini(&sess);
    TEST_END;
}
