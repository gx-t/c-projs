#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COUNT 1000000

struct SL_NODE
{
    struct SL_NODE* next;
    char str[0];
};

static void sl_add(const char* str, struct SL_NODE** head_ptr)
{
    struct SL_NODE* new_node = (struct SL_NODE*)malloc(sizeof(struct SL_NODE) + strlen(str) + 1);
    new_node->next = *head_ptr;
    strcpy(new_node->str, str);
    *head_ptr = new_node;
}

static void sl_free(struct SL_NODE** head_ptr)
{
    while(*head_ptr)
    {
        struct SL_NODE* next = (*head_ptr)->next;
        free(*head_ptr);
        *head_ptr = next;
    }
}

static struct SL_NODE* sl_tail(struct SL_NODE* node)
{
    if(!node)
        return 0;
    while(node->next)
        node = node->next;
    return node;
}

static struct SL_NODE* sl_partition(struct SL_NODE* head, struct SL_NODE* end, struct SL_NODE** new_head, struct SL_NODE** new_end)
{
    struct SL_NODE* pivot = end;
    struct SL_NODE* prev = 0;
    struct SL_NODE* curr = head;
    struct SL_NODE* tail = pivot;

    while(curr != pivot)
    {
        if(0 > strcmp(curr->str, pivot->str))
        {
            if(!*new_head)
                *new_head = curr;
            prev = curr;
            curr = curr->next;
        }
        else
        {
            if(prev)
                prev->next = curr->next;
            struct SL_NODE* tmp = curr->next;
            curr->next = 0;
            tail->next = curr;
            tail = curr;
            curr = tmp;
        }
    }

    if(!*new_head)
        *new_head = pivot;
    *new_end = tail;
    return pivot;
}

static struct SL_NODE* sl_quick_sort_r(struct SL_NODE* head, struct SL_NODE* end)
{
    if(!head || head == end)
        return head;

    struct SL_NODE* new_head = 0;
    struct SL_NODE* new_end = 0;

    struct SL_NODE* pivot = sl_partition(head, end, &new_head, &new_end);

    if(new_head != pivot)
    {
        struct SL_NODE* tmp = new_head;
        while(tmp->next != pivot)
            tmp = tmp->next;

        tmp->next = 0;

        new_head = sl_quick_sort_r(new_head, tmp);
        tmp = sl_tail(new_head);
        tmp->next = pivot;
    }

    pivot->next = sl_quick_sort_r(pivot->next, new_end);

    return new_head;
}

static void sl_quick_sort(struct SL_NODE** head_ptr)
{
    *head_ptr = sl_quick_sort_r(*head_ptr, sl_tail(*head_ptr));
}

static void sl_print(struct SL_NODE* node)
{
    for(; node; node = node->next)
        fprintf(stdout, "-- %s\n", node->str);
}

static void test_gen_random_str(char buff[64])
{
    const char arr[] = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int len = 3 + random() % 60;
    char* pp = buff;
    while(len --)
    {
        *pp ++ = arr[random() % sizeof(arr)];
    }
    *pp = 0;
}

static void test1()
{
    int count = 10000;
    fprintf(stderr, "===>>> test1: %d element SL list creation/deletion ... ", count);
    struct SL_NODE* list = 0;
    char str[64];
    int i = count;
    srandom(time(0));
    while(i--)
    {
        test_gen_random_str(str);
        sl_add(str, &list);
    }
    sl_free(&list);
}

static void test2()
{
    fprintf(stderr, "===>>> test2: %d element SL list quick sort ... ", COUNT);
    struct SL_NODE* list = 0;
    char str[64];
    int i = COUNT;
    srandom(time(0));
    while(i--)
    {
        test_gen_random_str(str);
        sl_add(str, &list);
    }
    clock_t clk = clock();
    sl_quick_sort(&list);
    fprintf(stderr, "%g sec\n", (float)(clock() - clk) / CLOCKS_PER_SEC);
    sl_free(&list);
}

static char str[COUNT][64];
static void test3()
{
    fprintf(stderr, "===>>> test3: %d element array quick sort ... ", COUNT);
    for(int i = 0; i < COUNT; i ++)
        test_gen_random_str(str[i]);
    clock_t clk = clock();
    qsort(str, COUNT, 64, (int (*)(const void*, const void*))strcmp);
    fprintf(stderr, "%g sec\n", (float)(clock() - clk) / CLOCKS_PER_SEC);
}

int main()
{
//    test1();
    test2();
    test3();
    return 0;
}

