/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020/12/31     Bernard      Add license info
 */

#include <stdint.h>
#include <rtthread.h>

/* ---------- Experiment 1: two threads add one shared variable ---------- */
static volatile rt_int32_t g_counter = 0;
static struct rt_mutex g_counter_lock;
static struct rt_semaphore g_counter_done;

static void counter_worker(void *parameter)
{
    rt_int32_t i;
    const char *name = (const char *)parameter;

    for (i = 0; i < 100; i++)
    {
        rt_mutex_take(&g_counter_lock, RT_WAITING_FOREVER);
        g_counter++;
        rt_mutex_release(&g_counter_lock);
    }

    rt_kprintf("[counter] %s finished.\n", name);
    rt_sem_release(&g_counter_done);
}

static void run_counter_experiment(void)
{
    rt_thread_t t1;
    rt_thread_t t2;

    g_counter = 0;
    rt_mutex_init(&g_counter_lock, "clk", RT_IPC_FLAG_FIFO);
    rt_sem_init(&g_counter_done, "cdone", 0, RT_IPC_FLAG_FIFO);

    t1 = rt_thread_create("cnt1", counter_worker, "cnt1", 2048, 18, 10);
    t2 = rt_thread_create("cnt2", counter_worker, "cnt2", 2048, 18, 10);
    RT_ASSERT(t1 != RT_NULL && t2 != RT_NULL);
    rt_thread_startup(t1);
    rt_thread_startup(t2);

    rt_sem_take(&g_counter_done, RT_TICK_PER_SECOND * 5);
    rt_sem_take(&g_counter_done, RT_TICK_PER_SECOND * 5);

    rt_kprintf("[counter] final=%d expect=200 -> %s\n",
               g_counter, (g_counter == 200) ? "PASS" : "FAIL");

    rt_sem_detach(&g_counter_done);
    rt_mutex_detach(&g_counter_lock);
}

/* ---------- Experiment 2: mailbox ping-pong (A<->B every 500ms) ---------- */
#define PINGPONG_ROUNDS 5
static struct rt_mailbox g_mb_a2b;
static struct rt_mailbox g_mb_b2a;
static rt_ubase_t g_mb_a2b_pool[8];
static rt_ubase_t g_mb_b2a_pool[8];
static struct rt_semaphore g_pingpong_done;

static void mailbox_thread_a(void *parameter)
{
    rt_uint32_t i;
    rt_ubase_t ack = 0;
    RT_UNUSED(parameter);

    for (i = 1; i <= PINGPONG_ROUNDS; i++)
    {
        rt_thread_mdelay(500);
        rt_mb_send(&g_mb_a2b, (rt_ubase_t)i);
        rt_kprintf("[mail] A->B send %d\n", i);
        if (rt_mb_recv(&g_mb_b2a, &ack, RT_TICK_PER_SECOND) == RT_EOK)
        {
            rt_kprintf("[mail] A<-B ack %d\n", (int)ack);
        }
    }
    rt_sem_release(&g_pingpong_done);
}

static void mailbox_thread_b(void *parameter)
{
    rt_uint32_t i;
    rt_ubase_t mail = 0;
    RT_UNUSED(parameter);

    for (i = 0; i < PINGPONG_ROUNDS; i++)
    {
        if (rt_mb_recv(&g_mb_a2b, &mail, RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_kprintf("[mail] B recv %d, reply now\n", (int)mail);
            rt_mb_send(&g_mb_b2a, mail + 1000);
        }
    }
    rt_sem_release(&g_pingpong_done);
}

static void run_mailbox_experiment(void)
{
    rt_thread_t ta;
    rt_thread_t tb;

    rt_mb_init(&g_mb_a2b, "a2b", g_mb_a2b_pool, sizeof(g_mb_a2b_pool) / sizeof(g_mb_a2b_pool[0]), RT_IPC_FLAG_FIFO);
    rt_mb_init(&g_mb_b2a, "b2a", g_mb_b2a_pool, sizeof(g_mb_b2a_pool) / sizeof(g_mb_b2a_pool[0]), RT_IPC_FLAG_FIFO);
    rt_sem_init(&g_pingpong_done, "ppdone", 0, RT_IPC_FLAG_FIFO);

    ta = rt_thread_create("mailA", mailbox_thread_a, RT_NULL, 2048, 17, 10);
    tb = rt_thread_create("mailB", mailbox_thread_b, RT_NULL, 2048, 17, 10);
    RT_ASSERT(ta != RT_NULL && tb != RT_NULL);
    rt_thread_startup(ta);
    rt_thread_startup(tb);

    rt_sem_take(&g_pingpong_done, RT_TICK_PER_SECOND * 10);
    rt_sem_take(&g_pingpong_done, RT_TICK_PER_SECOND * 10);

    rt_kprintf("[mail] ping-pong finished.\n");

    rt_sem_detach(&g_pingpong_done);
    rt_mb_detach(&g_mb_a2b);
    rt_mb_detach(&g_mb_b2a);
}

/* ---------- Experiment 3: soft interrupt(timer) sends periodic messages ---------- */
static struct rt_messagequeue g_softirq_mq;
static char g_softirq_mq_pool[16 * sizeof(rt_uint32_t)];
static struct rt_timer *g_softirq_timer = RT_NULL;
static struct rt_semaphore g_softirq_done;
static rt_uint32_t g_softirq_seq = 0;

static void softirq_timer_cb(void *parameter)
{
    rt_uint32_t msg = ++g_softirq_seq;
    rt_err_t ret;
    RT_UNUSED(parameter);
    ret = rt_mq_send(&g_softirq_mq, &msg, sizeof(msg));
    rt_kprintf("[softirq] timer send msg=%u ret=%d\n", msg, ret);
}

static void softirq_consumer(void *parameter)
{
    rt_int32_t i;
    rt_uint32_t msg;
    rt_ssize_t recv_len;
    const char *name = (const char *)parameter;

    for (i = 0; i < 5; i++)
    {
        recv_len = rt_mq_recv(&g_softirq_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (recv_len > 0)
        {
            rt_kprintf("[softirq] %s recv msg=%u\n", name, msg);
        }
        else
        {
            rt_kprintf("[softirq] %s recv failed ret=%d\n", name, (int)recv_len);
        }
    }

    rt_sem_release(&g_softirq_done);
}

static void run_softirq_experiment(void)
{
    rt_thread_t tc1;
    rt_thread_t tc2;

    g_softirq_seq = 0;
    rt_mq_init(&g_softirq_mq, "smq", g_softirq_mq_pool, sizeof(rt_uint32_t), sizeof(g_softirq_mq_pool), RT_IPC_FLAG_FIFO);
    rt_sem_init(&g_softirq_done, "sqdone", 0, RT_IPC_FLAG_FIFO);

    g_softirq_timer = rt_timer_create("stmr", softirq_timer_cb, RT_NULL,
                                      RT_TICK_PER_SECOND / 5,
                                      RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
    RT_ASSERT(g_softirq_timer != RT_NULL);

    tc1 = rt_thread_create("sc1", softirq_consumer, "sc1", 2048, 19, 10);
    tc2 = rt_thread_create("sc2", softirq_consumer, "sc2", 2048, 19, 10);
    RT_ASSERT(tc1 != RT_NULL && tc2 != RT_NULL);
    rt_thread_startup(tc1);
    rt_thread_startup(tc2);
    rt_timer_start(g_softirq_timer);

    rt_sem_take(&g_softirq_done, RT_TICK_PER_SECOND * 10);
    rt_sem_take(&g_softirq_done, RT_TICK_PER_SECOND * 10);

    rt_timer_stop(g_softirq_timer);
    rt_timer_delete(g_softirq_timer);
    g_softirq_timer = RT_NULL;

    rt_sem_detach(&g_softirq_done);
    rt_mq_detach(&g_softirq_mq);

    rt_kprintf("[softirq] dispatch finished.\n");
}

int main(void)
{
    rt_kprintf("==== RT-Thread Lab Start ====\n");
    run_counter_experiment();
    run_mailbox_experiment();
    run_softirq_experiment();
    rt_kprintf("==== RT-Thread Lab Done ====\n");
    return 0;
}
