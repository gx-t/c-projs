#include <stdio.h>
#include <string.h>
#include <openssl/x509.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/aes.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/asn1.h>
#include <openssl/err.h>

void f0_void_void()
{
    fprintf(stderr, "\n==>> %s\n", __func__);
}

int f1_int_int(int i)
{
    fprintf(stderr, "\n==>> %s: %d\n", __func__, i);
    return ++i;
}

const char* f2_pchar_pchar(const char* text)
{
    fprintf(stderr, "\n==>> %s: %s\n", __func__, text);
    return "constand char* return from the 'so' function is ok\n";
}

struct NODE
{
    char* name;
    int val;
    struct NODE* next;
};

const struct NODE* f3_node_node(const struct NODE* node, const char* name)
{
    fprintf(stderr, "\n==>> %s: 0x%p, %s\n", __func__, node, name);
    while(node)
    {
        fprintf(stderr, "-->>> %s\n", node->name);
        if(!strcmp(node->name, name))
            return node;
        node = node->next;
    }
    return node;
}

void f4_load_pem_file_print_subject_based_string(const char* file_path, const char* txt)
{
    fprintf(stderr, "\n==>> %s, %s, %s\n", __func__, file_path, txt);
    FILE* ff = fopen(file_path, "r");
    if(!ff)
    {
        perror(file_path);
        return;
    }
    X509 *x509 = PEM_read_X509(ff, NULL, NULL, NULL);
    fclose(ff);
    if(!x509)
    {
        fprintf(stderr, "ERROR. %s,%d\n", __FILE__, __LINE__);
        return;
    }
    do {
        const ASN1_OBJECT *paobj = NULL;
        X509_ALGOR_get0(&paobj, NULL, NULL, X509_get0_tbs_sigalg(x509));
        int nid = OBJ_obj2nid(paobj);
        if(NID_sha256WithRSAEncryption == nid)
            fprintf(stderr, "-->>> NID_sha256WithRSAEncryption ... OK\n");

        char* buff = NULL;
        BIO* stream = BIO_new(BIO_s_mem());
        BIO_printf(stream, "%s ",  txt);
        X509_NAME_print_ex(stream, X509_get_subject_name(x509), 0, XN_FLAG_ONELINE);
        BIO_puts(stream, " ");
        long ll = BIO_get_mem_data(stream, &buff);
        buff[ll - 1] = 0;
        fprintf(stderr, "-->>> %s\n", buff);
        BIO_free(stream);
    }while(0);
    X509_free(x509);
}


int f5_load_pem_string_check_2018_cert(const char* pem_str, uint32_t pem_len)
{
    fprintf(stderr, "\n==>> %s, ...\n", __func__);

    int res = -1;
    X509* x509 = NULL;
    X509_NAME* x509_name = NULL;
    int cn_idx = -1;
    X509_NAME_ENTRY* x509_ne = NULL;
    ASN1_STRING* asn1 = NULL;
    BIO* bio = BIO_new(BIO_s_mem());
    if(!bio)
    {
        fprintf(stderr, "ERROR. %s,%d\n", __FILE__, __LINE__);
        return -1;
    }
    do {
        if((pem_len != BIO_write(bio, pem_str, pem_len))
                || !(x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL))
                || !(x509_name = X509_get_subject_name(x509))
                || -1 == (cn_idx = X509_NAME_get_index_by_NID(x509_name, NID_commonName, -1))
                || !(x509_ne = X509_NAME_get_entry(x509_name, cn_idx))
                || !(asn1 = X509_NAME_ENTRY_get_data(x509_ne)))
        {
            fprintf(stderr, "ERROR. %s,%d\n", __FILE__, __LINE__);
            break;
        }
        const char* subj_cn = (const char*)ASN1_STRING_get0_data(asn1);
        if(!subj_cn)
        {
            res = 1;
            break;
        }
        fprintf(stderr, "Subject CN: %s\n", subj_cn);
        res = !strstr(subj_cn, "2018");
    }while(0);

    BIO_free(bio);
    X509_free(x509);
    return res;
}

void f6_call_callback(void (*f)(int, const char*), int i, const char* txt)
{
    const char* pref = "Called from C.... Call number %01d. Passed string is '%s'\n";
    char buff[strlen(txt) + strlen(pref) + 2];
    for(int c = 0; c < 8; c ++)
    {
        sprintf(buff, pref, c, txt);
        f(i + c, buff);
    }
}

int f7_int_node(const struct NODE** node_pp, const char* name)
{
    fprintf(stderr, "\n==>> %s: 0x%p, %s\n", __func__, node_pp, name);
    while(*node_pp)
    {
        fprintf(stderr, "-->>> %s\n", (*node_pp)->name);
        if(!strcmp((*node_pp)->name, name))
            return 0;
        *node_pp = (*node_pp)->next;
    }
    return -1;
}

int g_val_int = 37;

struct {
    void (*f0)();
    int (*f1)(int);
    const char* (*f2)(const char*);
}g_api = {
    .f0 = f0_void_void,
    .f1 = f1_int_int,
    .f2 = f2_pchar_pchar
};

int g_arr_int[] = {
    0, 1, 2, 3
};

int (*g_arr_func[])() = {
    f1_int_int, f1_int_int, f7_int_node
};
