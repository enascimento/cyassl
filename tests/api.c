/* api.c API unit tests
 *
 * Copyright (C) 2006-2011 Sawtooth Consulting Ltd.
 *
 * This file is part of CyaSSL.
 *
 * CyaSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * CyaSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdlib.h>
#include <cyassl/ssl.h>
#include <cyassl/test.h>
#include "unit.h"

#define TEST_FAIL       (-1)
#define TEST_SUCCESS    (0)

static int test_CyaSSL_Init(void);
static int test_CyaSSL_Cleanup(void);
static int test_CyaSSL_Method_Allocators(void);
static int test_CyaSSL_CTX_new(CYASSL_METHOD *method);
static int test_CyaSSL_CTX_use_certificate_file(void);
static int test_CyaSSL_CTX_use_PrivateKey_file(void);
static int test_CyaSSL_CTX_load_verify_locations(void);
static int test_server_CyaSSL_new(void);
static int test_client_CyaSSL_new(void);
#ifndef SINGLE_THREADED
static int test_CyaSSL_read_write(void);
#endif

/* test function helpers */
static int test_method(CYASSL_METHOD *method, const char *name);
static int test_method2(CYASSL_METHOD *method, const char *name);
static int test_ucf(CYASSL_CTX *ctx, const char* file, int type,
    int cond, const char* name);
static int test_upkf(CYASSL_CTX *ctx, const char* file, int type,
    int cond, const char* name);
static int test_lvl(CYASSL_CTX *ctx, const char* file, const char* path,
    int cond, const char* name);

#if 0
static const char* svrCert    = "./certs/server-cert.pem";
static const char* svrKey     = "./certs/server-key.pem";
#endif
static const char* bogusFile  = "/dev/null";
static const char* testingFmt = "   %s:";
static const char* resultFmt  = " %s\n";
static const char* passed     = "passed";
static const char* failed     = "failed";

/* List of methods found in echoserver.c that I'm skipping for the moment:
 * - CyaSSL_CTX_set_session_cache_mode()
 */

int ApiTest(void)
{
    printf(" Begin API Tests\n");
    test_CyaSSL_Init();
    test_CyaSSL_Method_Allocators();
    test_CyaSSL_CTX_new(CyaSSLv23_server_method());
    test_CyaSSL_CTX_use_certificate_file();
    test_CyaSSL_CTX_use_PrivateKey_file();
    test_CyaSSL_CTX_load_verify_locations();
    test_server_CyaSSL_new();
    test_client_CyaSSL_new();
#ifndef SINGLE_THREADED
    test_CyaSSL_read_write();
#endif
    test_CyaSSL_Cleanup();
    printf(" End API Tests\n");

    return TEST_SUCCESS;
}

int test_CyaSSL_Init(void)
{
    int result;

    printf(testingFmt, "CyaSSL_Init()");
    result = CyaSSL_Init();
    printf(resultFmt, result ? failed : passed);

    return result;
}

static int test_CyaSSL_Cleanup(void)
{
    int result;

    printf(testingFmt, "CyaSSL_Cleanup()");
    result = CyaSSL_Cleanup();
    printf(resultFmt, result ? failed : passed);

    return result;
}

int test_method(CYASSL_METHOD *method, const char *name)
{
    printf(testingFmt, name);
    if (method == NULL)
    {
        printf(resultFmt, failed);
        return TEST_FAIL;
    }
    XFREE(method, 0, DYNAMIC_TYPE_METHOD);
    printf(resultFmt, passed);
    return TEST_SUCCESS;
}

int test_method2(CYASSL_METHOD *method, const char *name)
{
    printf(testingFmt, name);
    if (method != NULL)
    {
        XFREE(method, 0, DYNAMIC_TYPE_METHOD);
        printf(resultFmt, failed);
        return TEST_FAIL;
    }
    printf(resultFmt, passed);
    return TEST_SUCCESS;
}

int test_CyaSSL_Method_Allocators(void)
{
    test_method(CyaSSLv3_server_method(), "CyaSSLv3_server_method()");
    test_method(CyaSSLv3_client_method(), "CyaSSLv3_client_method()");
    test_method(CyaTLSv1_server_method(), "CyaTLSv1_server_method()");
    test_method(CyaTLSv1_client_method(), "CyaTLSv1_client_method()");
    test_method(CyaTLSv1_1_server_method(), "CyaTLSv1_1_server_method()");
    test_method(CyaTLSv1_1_client_method(), "CyaTLSv1_1_client_method()");
    test_method(CyaTLSv1_2_server_method(), "CyaTLSv1_2_server_method()");
    test_method(CyaTLSv1_2_client_method(), "CyaTLSv1_2_client_method()");
    test_method(CyaSSLv23_client_method(), "CyaSSLv23_client_method()");

#ifdef CYASSL_DTLS
    test_method(CyaDTLSv1_server_method(), "CyaDTLSv1_server_method()");
    test_method(CyaDTLSv1_client_method(), "CyaDTLSv1_client_method()");
#endif /* CYASSL_DTLS */

#ifdef OPENSSL_EXTRA
    test_method2(CyaSSLv2_server_method(), "CyaSSLv2_server_method()");
    test_method2(CyaSSLv2_client_method(), "CyaSSLv2_client_method()");
#endif /* OPENSSL_EXTRA */

    return TEST_SUCCESS;
}

int test_CyaSSL_CTX_new(CYASSL_METHOD *method)
{
    if (method != NULL)
    {
        CYASSL_CTX *ctx;
    
        printf(testingFmt, "CyaSSL_CTX_new(NULL)");
        ctx = CyaSSL_CTX_new(NULL);
        if (ctx != NULL)
        {
            CyaSSL_CTX_free(ctx);
            printf(resultFmt, failed);
        }
        else
            printf(resultFmt, passed);
    
        printf(testingFmt, "CyaSSL_CTX_new(method)");
        ctx = CyaSSL_CTX_new(method);
        if (ctx == NULL)
        {
            printf(resultFmt, failed);
            XFREE(method, 0, DYNAMIC_TYPE_METHOD);
            /* free the method data. if this was successful, freeing
               the CTX frees the method. */
        }
        else
        {
            CyaSSL_CTX_free(ctx);
            printf(resultFmt, passed);
        }
    }
    else
        printf("test_CyaSSL_CTX_new() called without method\n");

    return TEST_SUCCESS;
}

/* Helper for testing CyaSSL_CTX_use_certificate_file() */
int test_ucf(CYASSL_CTX *ctx, const char* file, int type, int cond,
    const char* name)
{
    int result;

    printf(testingFmt, name);
    result = CyaSSL_CTX_use_certificate_file(ctx, file, type);
    if (result != cond)
    {
        printf(resultFmt, failed);
        return TEST_FAIL;
    }
    printf(resultFmt, passed);
    return TEST_SUCCESS;
}

int test_CyaSSL_CTX_use_certificate_file(void)
{
    CYASSL_METHOD *method;
    CYASSL_CTX *ctx;

    method = CyaSSLv23_server_method();
    if (method == NULL)
    {
        printf("test_CyaSSL_CTX_use_certificate_file() cannot create method\n");
        return TEST_FAIL;
    }

    ctx = CyaSSL_CTX_new(method);
    if (ctx == NULL)
    {
        printf("test_CyaSSL_CTX_use_certificate_file() cannot create context\n");
        XFREE(method, 0, DYNAMIC_TYPE_METHOD);
        return TEST_FAIL;
    }

    /* setting all parameters to garbage. this should succeed with
        failure */
    /* Then set the parameters to legit values but set each item to
        bogus and call again. Finish with a successful success. */

    test_ucf(NULL, NULL, 9999, SSL_FAILURE,
        "CyaSSL_CTX_use_certificate_file(NULL, NULL, 9999)");
/*  test_ucf(NULL, svrCert, SSL_FILETYPE_PEM, SSL_FAILURE,
        "CyaSSL_CTX_use_certificate_file(NULL, svrCert, SSL_FILETYPE_PEM)");*/
    test_ucf(ctx, bogusFile, SSL_FILETYPE_PEM, SSL_FAILURE,
        "CyaSSL_CTX_use_certificate_file(ctx, bogusFile, SSL_FILETYPE_PEM)");
    test_ucf(ctx, svrCert, 9999, SSL_FAILURE,
        "CyaSSL_CTX_use_certificate_file(ctx, svrCert, 9999)");
    test_ucf(ctx, svrCert, SSL_FILETYPE_PEM, SSL_SUCCESS,
        "CyaSSL_CTX_use_certificate_file(ctx, svrCert, SSL_FILETYPE_PEM)");

    CyaSSL_CTX_free(ctx);
    return TEST_SUCCESS;
}

/* Helper for testing CyaSSL_CTX_use_PrivateKey_file() */
int test_upkf(CYASSL_CTX *ctx, const char* file, int type, int cond,
    const char* name)
{
    int result;

    printf(testingFmt, name);
    result = CyaSSL_CTX_use_PrivateKey_file(ctx, file, type);
    if (result != cond)
    {
        printf(resultFmt, failed);
        return TEST_FAIL;
    }
    printf(resultFmt, passed);
    return TEST_SUCCESS;
}

int test_CyaSSL_CTX_use_PrivateKey_file(void)
{
    CYASSL_METHOD *method;
    CYASSL_CTX *ctx;

    method = CyaSSLv23_server_method();
    if (method == NULL)
    {
        printf("test_CyaSSL_CTX_use_PrivateKey_file() cannot create method\n");
        return TEST_FAIL;
    }

    ctx = CyaSSL_CTX_new(method);
    if (ctx == NULL)
    {
        printf("test_CyaSSL_CTX_use_PrivateKey_file() cannot create context\n");
        XFREE(method, 0, DYNAMIC_TYPE_METHOD);
        return TEST_FAIL;
    }

    test_upkf(NULL, NULL, 9999, SSL_FAILURE,
        "CyaSSL_CTX_use_PrivateKey_file(NULL, NULL, 9999)");
/*  test_upkf(NULL, svrKey, SSL_FILETYPE_PEM, SSL_FAILURE,
        "CyaSSL_CTX_use_PrivateKey_file(NULL, svrKey, SSL_FILETYPE_PEM)");*/
    test_upkf(ctx, bogusFile, SSL_FILETYPE_PEM, SSL_FAILURE,
        "CyaSSL_CTX_use_PrivateKey_file(ctx, bogusFile, SSL_FILETYPE_PEM)");
    test_upkf(ctx, svrKey, 9999, SSL_FAILURE,
        "CyaSSL_CTX_use_PrivateKey_file(ctx, svrKey, 9999)");
    test_upkf(ctx, svrKey, SSL_FILETYPE_PEM, SSL_SUCCESS,
        "CyaSSL_CTX_use_PrivateKey_file(ctx, svrKey, SSL_FILETYPE_PEM)");

    CyaSSL_CTX_free(ctx);
    return TEST_SUCCESS;
}

/* Helper for testing CyaSSL_CTX_load_verify_locations() */
int test_lvl(CYASSL_CTX *ctx, const char* file, const char* path, int cond,
    const char* name)
{
    int result;

    printf(testingFmt, name);
    result = CyaSSL_CTX_load_verify_locations(ctx, file, path);
    if (result != cond)
    {
        printf(resultFmt, failed);
        return TEST_FAIL;
    }
    printf(resultFmt, passed);
    return TEST_SUCCESS;
}

int test_CyaSSL_CTX_load_verify_locations(void)
{
    CYASSL_METHOD *method;
    CYASSL_CTX *ctx;

    method = CyaSSLv23_client_method();
    if (method == NULL)
    {
        printf("test_CyaSSL_CTX_load_verify_locations() cannot create method\n");
        return TEST_FAIL;
    }

    ctx = CyaSSL_CTX_new(method);
    if (ctx == NULL)
    {
        printf("test_CyaSSL_CTX_load_verify_locations() cannot create context\n");
        free(method);
        return TEST_FAIL;
    }
    
    test_lvl(NULL, NULL, NULL, SSL_FAILURE,
        "CyaSSL_CTX_load_verify_locations(NULL, NULL, NULL)");
    test_lvl(ctx, NULL, NULL, SSL_FAILURE,
        "CyaSSL_CTX_load_verify_locations(ctx, NULL, NULL)");
    test_lvl(NULL, caCert, NULL, SSL_FAILURE,
        "CyaSSL_CTX_load_verify_locations(ctx, NULL, NULL)");
    test_lvl(ctx, caCert, bogusFile, SSL_SUCCESS,
        "CyaSSL_CTX_load_verify_locations(ctx, caCert, bogusFile)");
    /* There is a leak here. If you load a second cert, the first one
       is lost. */
    test_lvl(ctx, caCert, 0, SSL_SUCCESS,
        "CyaSSL_CTX_load_verify_locations(ctx, caCert, 0)");

    CyaSSL_CTX_free(ctx);
    return TEST_SUCCESS;
}

int test_server_CyaSSL_new(void)
{
    int result;
    CYASSL_CTX *ctx;
    CYASSL_CTX *ctx_nocert;
    CYASSL *ssl;

    ctx = CyaSSL_CTX_new(CyaSSLv23_server_method());
    if (ctx == NULL)
    {
        printf("test_server_CyaSSL_new() cannot create context\n");
        return TEST_FAIL;
    }

    result = CyaSSL_CTX_use_certificate_file(ctx, svrCert, SSL_FILETYPE_PEM);
    if (result == SSL_FAILURE)
    {
        printf("test_server_CyaSSL_new() cannot obtain certificate\n");
        CyaSSL_CTX_free(ctx);
        return TEST_FAIL;
    }

    result = CyaSSL_CTX_use_PrivateKey_file(ctx, svrKey, SSL_FILETYPE_PEM);
    if (result == SSL_FAILURE)
    {
        printf("test_server_CyaSSL_new() cannot obtain key\n");
        CyaSSL_CTX_free(ctx);
        return TEST_FAIL;
    }

    ctx_nocert = CyaSSL_CTX_new(CyaSSLv23_server_method());
    if (ctx_nocert == NULL)
    {
        printf("test_server_CyaSSL_new() cannot create bogus context\n");
        CyaSSL_CTX_free(ctx);
        return TEST_FAIL;
    }

    printf(testingFmt, "CyaSSL_new(NULL) server");
    ssl = CyaSSL_new(NULL);
    if (ssl != NULL)
    {
        printf(resultFmt, failed);
        CyaSSL_free(ssl);
    }
    else
        printf(resultFmt, passed);

    printf(testingFmt, "CyaSSL_new(ctx_nocert) server");
    ssl = CyaSSL_new(ctx_nocert);
    if (ssl != NULL)
    {
        printf(resultFmt, failed);
        CyaSSL_free(ssl);
    }
    else
        printf(resultFmt, passed);

    printf(testingFmt, "CyaSSL_new(ctx) server");
    ssl = CyaSSL_new(ctx);
    if (ssl == NULL)
        printf(resultFmt, failed);
    else
    {
        printf(resultFmt, passed);
        CyaSSL_free(ssl);
    }
    
    CyaSSL_CTX_free(ctx_nocert);
    CyaSSL_CTX_free(ctx);
    return TEST_SUCCESS;
}

int test_client_CyaSSL_new(void)
{
    int result;
    CYASSL_CTX *ctx;
    CYASSL_CTX *ctx_nocert;
    CYASSL *ssl;

    ctx = CyaSSL_CTX_new(CyaSSLv23_client_method());
    if (ctx == NULL)
    {
        printf("test_client_CyaSSL_new() cannot create context\n");
        return TEST_FAIL;
    }

    result = CyaSSL_CTX_load_verify_locations(ctx, caCert, 0);
    if (result == SSL_FAILURE)
    {
        printf("test_client_CyaSSL_new() cannot obtain certificate\n");
        CyaSSL_CTX_free(ctx);
        return TEST_FAIL;
    }

    ctx_nocert = CyaSSL_CTX_new(CyaSSLv23_client_method());
    if (ctx_nocert == NULL)
    {
        printf("test_client_CyaSSL_new() cannot create bogus context\n");
        CyaSSL_CTX_free(ctx);
        return TEST_FAIL;
    }

    printf(testingFmt, "CyaSSL_new(NULL) client");
    ssl = CyaSSL_new(NULL);
    if (ssl != NULL)
    {
        printf(resultFmt, failed);
        CyaSSL_free(ssl);
    }
    else
        printf(resultFmt, passed);

    printf(testingFmt, "CyaSSL_new(ctx_nocert) client");
    ssl = CyaSSL_new(ctx_nocert);
    if (ssl == NULL)
    {
        printf(resultFmt, failed);
        CyaSSL_free(ssl);
    }
    else
        printf(resultFmt, passed);

    printf(testingFmt, "CyaSSL_new(ctx) client");
    ssl = CyaSSL_new(ctx);
    if (ssl == NULL)
        printf(resultFmt, failed);
    else
    {
        printf(resultFmt, passed);
        CyaSSL_free(ssl);
    }
    
    CyaSSL_CTX_free(ctx_nocert);
    CyaSSL_CTX_free(ctx);
    return TEST_SUCCESS;
}

#ifndef SINGLE_THREADED
static int test_CyaSSL_read_write(void)
{
    printf(testingFmt, "read and write");
    printf(resultFmt, "undefined");
    return TEST_SUCCESS;
};
#endif