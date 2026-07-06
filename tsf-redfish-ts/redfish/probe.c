/** @file
 * @brief Redfish Group
 *
 * Fetch a Redfish service root and log its identity. The endpoint is the
 * test's to supply (env TSF_REDFISH_URL, e.g. "https://bmc"); with none
 * configured the test skips cleanly - a host with no BMC to point at is
 * not a failure. Read-only; no power or reset actions.
 *
 * Copyright (C) 2026 Interpretica Unipessoal Lda
 */

#define TE_TEST_NAME    "redfish/probe"

#include "te_config.h"
#include <stdlib.h>
#include "tapi_test.h"
#include "te_string.h"

#include "tapi_redfish.h"
#include "tsapi_redfish.h"

int
main(int argc, char **argv)
{
    tsapi_redfish_session sess = {0};
    tapi_redfish_root root;
    bool root_ready = false;
    const char *url;
    const char *user;
    const char *pass;
    int http_code = 0;

    TEST_START;

    url = getenv("TSF_REDFISH_URL");
    if (url == NULL || url[0] == '\0')
        TEST_SKIP("Set TSF_REDFISH_URL (e.g. https://bmc) to point at a service");
    user = getenv("TSF_REDFISH_USER");
    pass = getenv("TSF_REDFISH_PASS");

    TEST_STEP("Open a session to the agent");
    CHECK_RC(tsapi_redfish_session_init(&sess, "pco_redfish_probe"));

    TEST_STEP("GET the Redfish service root of %s", url);
    CHECK_RC(tapi_redfish_get_root(sess.pco, url, user, pass, &root,
                                   &http_code));
    root_ready = true;
    RING("service root HTTP %d (%s)", http_code, root.secure ? "https" : "http");
    tapi_redfish_root_log(&root);

    TEST_STEP("The service answered as Redfish");
    if (http_code != 200 && http_code != 401)
        TEST_VERDICT("unexpected HTTP %d from the Redfish root", http_code);

    TEST_SUCCESS;

cleanup:
    if (root_ready)
        tapi_redfish_root_free(&root);
    tsapi_redfish_session_fini(&sess);
    TEST_END;
}
