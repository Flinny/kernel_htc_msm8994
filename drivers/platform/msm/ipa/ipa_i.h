/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _IPA_I_H_
#define _IPA_I_H_

#include <linux/bitops.h>
#include <linux/cdev.h>
#include <linux/export.h>
#include <linux/idr.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/ipa.h>
#include <linux/msm-sps.h>
#include "ipa_hw_defs.h"
#include "ipa_ram_mmap.h"
#include "ipa_reg.h"

#define DRV_NAME "ipa"
#define NAT_DEV_NAME "ipaNatTable"
#define IPA_COOKIE 0x57831603
#define MTU_BYTE 1500

#define IPA_NUM_PIPES 0x14
#define IPA_SYS_DESC_FIFO_SZ 0x800
#define IPA_SYS_TX_DATA_DESC_FIFO_SZ 0x1000
#define IPA_LAN_RX_HEADER_LENGTH (2)
#define IPA_QMAP_HEADER_LENGTH (4)
#define IPA_DL_CHECKSUM_LENGTH (8)
#define IPA_NUM_DESC_PER_SW_TX (2)

#define IPADBG(fmt, args...) \
	pr_debug(DRV_NAME " %s:%d " fmt, __func__, __LINE__, ## args)
#define IPAERR(fmt, args...) \
	pr_err(DRV_NAME " %s:%d " fmt, __func__, __LINE__, ## args)

#define WLAN_AMPDU_TX_EP 15
#define WLAN_PROD_TX_EP  19
#define WLAN1_CONS_RX_EP  14
#define WLAN2_CONS_RX_EP  16
#define WLAN3_CONS_RX_EP  17
#define WLAN4_CONS_RX_EP  18

#define MAX_NUM_EXCP     8

#define IPA_STATS

#ifdef IPA_STATS
#define IPA_STATS_INC_CNT(val) do {			\
				++val;			\
			} while (0)
#define IPA_STATS_DEC_CNT(val) (--val)
#define IPA_STATS_EXCP_CNT(flags, base) do {			\
			int i;					\
			for (i = 0; i < MAX_NUM_EXCP; i++)	\
				if (flags & BIT(i))		\
					++base[i];		\
			if (flags == 0)				\
				++base[MAX_NUM_EXCP - 1];	\
			} while (0)
#else
#define IPA_STATS_INC_CNT(x) do { } while (0)
#define IPA_STATS_DEC_CNT(x)
#define IPA_STATS_EXCP_CNT(flags, base) do { } while (0)
#endif

#define IPA_TOS_EQ			BIT(0)
#define IPA_PROTOCOL_EQ			BIT(1)
#define IPA_OFFSET_MEQ32_0		BIT(2)
#define IPA_OFFSET_MEQ32_1		BIT(3)
#define IPA_IHL_OFFSET_RANGE16_0	BIT(4)
#define IPA_IHL_OFFSET_RANGE16_1	BIT(5)
#define IPA_IHL_OFFSET_EQ_16		BIT(6)
#define IPA_IHL_OFFSET_EQ_32		BIT(7)
#define IPA_IHL_OFFSET_MEQ32_0		BIT(8)
#define IPA_OFFSET_MEQ128_0		BIT(9)
#define IPA_OFFSET_MEQ128_1		BIT(10)
#define IPA_TC_EQ			BIT(11)
#define IPA_FL_EQ			BIT(12)
#define IPA_IHL_OFFSET_MEQ32_1		BIT(13)
#define IPA_METADATA_COMPARE		BIT(14)
#define IPA_IS_FRAG			BIT(15)

#define IPA_HDR_BIN0 0
#define IPA_HDR_BIN1 1
#define IPA_HDR_BIN2 2
#define IPA_HDR_BIN3 3
#define IPA_HDR_BIN4 4
#define IPA_HDR_BIN_MAX 5

#define IPA_HDR_PROC_CTX_BIN0 0
#define IPA_HDR_PROC_CTX_BIN1 1
#define IPA_HDR_PROC_CTX_BIN_MAX 2

#define IPA_EVENT_THRESHOLD 0x10

#define IPA_USB_EVENT_THRESHOLD 0x4001

#define IPA_RX_POOL_CEIL 32
#define IPA_RX_SKB_SIZE 1792

#define IPA_A5_MUX_HDR_NAME "ipa_excp_hdr"
#define IPA_LAN_RX_HDR_NAME "ipa_lan_hdr"
#define IPA_INVALID_L4_PROTOCOL 0xFF

#define IPA_CLIENT_IS_PROD(x) (x >= IPA_CLIENT_PROD && x < IPA_CLIENT_CONS)
#define IPA_CLIENT_IS_CONS(x) (x >= IPA_CLIENT_CONS && x < IPA_CLIENT_MAX)
#define IPA_SETFIELD(val, shift, mask) (((val) << (shift)) & (mask))
#define IPA_SETFIELD_IN_REG(reg, val, shift, mask) \
			(reg |= ((val) << (shift)) & (mask))

#define IPA_HW_TABLE_ALIGNMENT(start_ofst) \
	(((start_ofst) + 127) & ~127)
#define IPA_RT_FLT_HW_RULE_BUF_SIZE	(128)

#define IPA_HDR_PROC_CTX_TABLE_ALIGNMENT_BYTE 8
#define IPA_HDR_PROC_CTX_TABLE_ALIGNMENT(start_ofst) \
	(((start_ofst) + IPA_HDR_PROC_CTX_TABLE_ALIGNMENT_BYTE - 1) & \
	~(IPA_HDR_PROC_CTX_TABLE_ALIGNMENT_BYTE - 1))

#define MAX_RESOURCE_TO_CLIENTS (5)
#define IPA_MEM_PART(x_) (ipa_ctx->ctrl->mem_partition.x_)

struct ipa_client_names {
	enum ipa_client_type names[MAX_RESOURCE_TO_CLIENTS];
	int length;
};

struct ipa_mem_buffer {
	void *base;
	dma_addr_t phys_base;
	u32 size;
};

struct ipa_flt_entry {
	struct list_head link;
	struct ipa_flt_rule rule;
	u32 cookie;
	struct ipa_flt_tbl *tbl;
	struct ipa_rt_tbl *rt_tbl;
	u32 hw_len;
	int id;
};

struct ipa_rt_tbl {
	struct list_head link;
	struct list_head head_rt_rule_list;
	char name[IPA_RESOURCE_NAME_MAX];
	u32 idx;
	u32 rule_cnt;
	u32 ref_cnt;
	struct ipa_rt_tbl_set *set;
	u32 cookie;
	bool in_sys;
	u32 sz;
	struct ipa_mem_buffer curr_mem;
	struct ipa_mem_buffer prev_mem;
	int id;
};

struct ipa_hdr_entry {
	struct list_head link;
	u8 hdr[IPA_HDR_MAX_SIZE];
	u32 hdr_len;
	char name[IPA_RESOURCE_NAME_MAX];
	enum ipa_hdr_l2_type type;
	u8 is_partial;
	bool is_hdr_proc_ctx;
	dma_addr_t phys_base;
	struct ipa_hdr_proc_ctx_entry *proc_ctx;
	struct ipa_hdr_offset_entry *offset_entry;
	u32 cookie;
	u32 ref_cnt;
	int id;
	u8 is_eth2_ofst_valid;
	u16 eth2_ofst;
};

struct ipa_hdr_offset_entry {
	struct list_head link;
	u32 offset;
	u32 bin;
};

struct ipa_hdr_tbl {
	struct list_head head_hdr_entry_list;
	struct list_head head_offset_list[IPA_HDR_BIN_MAX];
	struct list_head head_free_offset_list[IPA_HDR_BIN_MAX];
	u32 hdr_cnt;
	u32 end;
};

struct ipa_hdr_proc_ctx_offset_entry {
	struct list_head link;
	u32 offset;
	u32 bin;
};

struct ipa_hdr_proc_ctx_add_hdr_seq {
	struct ipa_hdr_proc_ctx_hdr_add hdr_add;
	struct ipa_hdr_proc_ctx_tlv end;
};

struct ipa_hdr_proc_ctx_add_hdr_cmd_seq {
	struct ipa_hdr_proc_ctx_hdr_add hdr_add;
	struct ipa_hdr_proc_ctx_tlv cmd;
	struct ipa_hdr_proc_ctx_tlv end;
};

struct ipa_hdr_proc_ctx_entry {
	struct list_head link;
	enum ipa_hdr_proc_type type;
	struct ipa_hdr_proc_ctx_offset_entry *offset_entry;
	struct ipa_hdr_entry *hdr;
	u32 cookie;
	u32 ref_cnt;
	int id;
};

struct ipa_hdr_proc_ctx_tbl {
	struct list_head head_proc_ctx_entry_list;
	struct list_head head_offset_list[IPA_HDR_PROC_CTX_BIN_MAX];
	struct list_head head_free_offset_list[IPA_HDR_PROC_CTX_BIN_MAX];
	u32 proc_ctx_cnt;
	u32 end;
	u32 start_offset;
};

struct ipa_flt_tbl {
	struct list_head head_flt_rule_list;
	u32 rule_cnt;
	bool in_sys;
	u32 sz;
	struct ipa_mem_buffer curr_mem;
	struct ipa_mem_buffer prev_mem;
	bool sticky_rear;
};

struct ipa_rt_entry {
	struct list_head link;
	struct ipa_rt_rule rule;
	u32 cookie;
	struct ipa_rt_tbl *tbl;
	struct ipa_hdr_entry *hdr;
	struct ipa_hdr_proc_ctx_entry *proc_ctx;
	u32 hw_len;
	int id;
};

struct ipa_rt_tbl_set {
	struct list_head head_rt_tbl_list;
	u32 tbl_cnt;
};

struct ipa_ep_cfg_status {
	bool status_en;
	u8 status_ep;
};

struct ipa_wlan_stats {
	u32 rx_pkts_rcvd;
	u32 rx_pkts_status_rcvd;
	u32 rx_hd_processed;
	u32 rx_hd_reply;
	u32 rx_hd_rcvd;
	u32 rx_pkt_leak;
	u32 rx_dp_fail;
	u32 tx_pkts_rcvd;
	u32 tx_pkts_sent;
	u32 tx_pkts_dropped;
};

struct ipa_wlan_comm_memb {
	spinlock_t wlan_spinlock;
	spinlock_t ipa_tx_mul_spinlock;
	u32 wlan_comm_total_cnt;
	u32 wlan_comm_free_cnt;
	u32 total_tx_pkts_freed;
	struct list_head wlan_comm_desc_list;
	atomic_t active_clnt_cnt;
};

struct ipa_ep_context {
	int valid;
	enum ipa_client_type client;
	struct sps_pipe *ep_hdl;
	struct ipa_ep_cfg cfg;
	struct ipa_ep_cfg_holb holb;
	struct ipa_ep_cfg_status status;
	u32 dst_pipe_index;
	u32 rt_tbl_idx;
	struct sps_connect connect;
	void *priv;
	void (*client_notify)(void *priv, enum ipa_dp_evt_type evt,
		       unsigned long data);
	bool desc_fifo_in_pipe_mem;
	bool data_fifo_in_pipe_mem;
	u32 desc_fifo_pipe_mem_ofst;
	u32 data_fifo_pipe_mem_ofst;
	bool desc_fifo_client_allocated;
	bool data_fifo_client_allocated;
	atomic_t avail_fifo_desc;
	u32 dflt_flt4_rule_hdl;
	u32 dflt_flt6_rule_hdl;
	bool skip_ep_cfg;
	bool keep_ipa_awake;
	struct ipa_wlan_stats wstats;
	u32 wdi_state;

	
	struct ipa_sys_context *sys;
};

enum ipa_sys_pipe_policy {
	IPA_POLICY_INTR_MODE,
	IPA_POLICY_NOINTR_MODE,
	IPA_POLICY_INTR_POLL_MODE,
};

struct ipa_repl_ctx {
	struct ipa_rx_pkt_wrapper **cache;
	atomic_t head_idx;
	atomic_t tail_idx;
	u32 capacity;
};

struct ipa_sys_context {
	u32 len;
	struct sps_register_event event;
	atomic_t curr_polling_state;
	struct delayed_work switch_to_intr_work;
	enum ipa_sys_pipe_policy policy;
	int (*pyld_hdlr)(struct sk_buff *skb, struct ipa_sys_context *sys);
	struct sk_buff *(*get_skb)(unsigned int len, gfp_t flags);
	void (*free_skb)(struct sk_buff *skb);
	u32 rx_buff_sz;
	u32 rx_pool_sz;
	struct sk_buff *prev_skb;
	unsigned int len_rem;
	unsigned int len_pad;
	unsigned int len_partial;
	struct work_struct work;
	void (*sps_callback)(struct sps_event_notify *notify);
	enum sps_option sps_option;
	struct delayed_work replenish_rx_work;
	struct work_struct repl_work;
	void (*repl_hdlr)(struct ipa_sys_context *sys);
	struct ipa_repl_ctx repl;

	
	struct ipa_ep_context *ep;
	struct list_head head_desc_list;
	spinlock_t spinlock;
	struct workqueue_struct *wq;
	struct workqueue_struct *repl_wq;
	
};

enum ipa_desc_type {
	IPA_DATA_DESC,
	IPA_DATA_DESC_SKB,
	IPA_IMM_CMD_DESC
};

struct ipa_tx_pkt_wrapper {
	enum ipa_desc_type type;
	struct ipa_mem_buffer mem;
	struct work_struct work;
	struct list_head link;
	void (*callback)(void *user1, int user2);
	void *user1;
	int user2;
	struct ipa_sys_context *sys;
	struct ipa_mem_buffer mult;
	u32 cnt;
	void *bounce;
	bool no_unmap_dma;
};

struct ipa_desc {
	enum ipa_desc_type type;
	void *pyld;
	dma_addr_t dma_address;
	bool dma_address_valid;
	u16 len;
	u16 opcode;
	void (*callback)(void *user1, int user2);
	void *user1;
	int user2;
	struct completion xfer_done;
};

struct ipa_rx_pkt_wrapper {
	struct list_head link;
	struct ipa_rx_data data;
	u32 len;
	struct work_struct work;
	struct ipa_sys_context *sys;
};

struct ipa_nat_mem {
	struct class *class;
	struct device *dev;
	struct cdev cdev;
	dev_t dev_num;
	void *vaddr;
	dma_addr_t dma_handle;
	size_t size;
	bool is_mapped;
	bool is_sys_mem;
	bool is_dev_init;
	bool is_dev;
	struct mutex lock;
	void *nat_base_address;
	char *ipv4_rules_addr;
	char *ipv4_expansion_rules_addr;
	char *index_table_addr;
	char *index_table_expansion_addr;
	u32 size_base_tables;
	u32 size_expansion_tables;
	u32 public_ip_addr;
	void *tmp_vaddr;
	dma_addr_t tmp_dma_handle;
	bool is_tmp_mem;
};

enum ipa_hw_mode {
	IPA_HW_MODE_NORMAL  = 0,
	IPA_HW_MODE_VIRTUAL = 1,
	IPA_HW_MODE_PCIE    = 2
};

enum ipa_config_this_ep {
	IPA_CONFIGURE_THIS_EP,
	IPA_DO_NOT_CONFIGURE_THIS_EP,
};

struct ipa_stats {
	u32 tx_sw_pkts;
	u32 tx_hw_pkts;
	u32 rx_pkts;
	u32 rx_excp_pkts[MAX_NUM_EXCP];
	u32 rx_repl_repost;
	u32 tx_pkts_compl;
	u32 rx_q_len;
	u32 msg_w[IPA_EVENT_MAX_NUM];
	u32 msg_r[IPA_EVENT_MAX_NUM];
	u32 stat_compl;
	u32 aggr_close;
	u32 wan_aggr_close;
	u32 wan_rx_empty;
	u32 wan_repl_rx_empty;
	u32 lan_rx_empty;
	u32 lan_repl_rx_empty;
};

struct ipa_active_clients {
	struct mutex mutex;
	spinlock_t spinlock;
	bool mutex_locked;
	int cnt;
};

struct ipa_tag_completion {
	struct completion comp;
	atomic_t cnt;
};

struct ipa_controller;

struct ipa_uc_ctx {
	bool uc_inited;
	bool uc_loaded;
	bool uc_failed;
	struct mutex uc_lock;
	struct completion uc_completion;
	struct IpaHwSharedMemCommonMapping_t *uc_sram_mmio;
	u32 pending_cmd;
	u32 uc_status;
	
	struct dma_pool *wdi_dma_pool;
	u32 wdi_uc_top_ofst;
	struct IpaHwEventLogInfoData_t *wdi_uc_top_mmio;
	u32 wdi_uc_stats_ofst;
	struct IpaHwStatsWDIInfoData_t *wdi_uc_stats_mmio;
};

struct ipa_sps_pm {
	spinlock_t lock;
	bool res_granted;
	bool res_rel_in_prog;
};

struct ipa_context {
	struct class *class;
	dev_t dev_num;
	struct device *dev;
	struct cdev cdev;
	unsigned long bam_handle;
	struct ipa_ep_context ep[IPA_NUM_PIPES];
	bool skip_ep_cfg_shadow[IPA_NUM_PIPES];
	bool resume_on_connect[IPA_CLIENT_MAX];
	struct ipa_flt_tbl flt_tbl[IPA_NUM_PIPES][IPA_IP_MAX];
	void __iomem *mmio;
	u32 ipa_wrapper_base;
	struct ipa_flt_tbl glob_flt_tbl[IPA_IP_MAX];
	struct ipa_hdr_tbl hdr_tbl;
	struct ipa_hdr_proc_ctx_tbl hdr_proc_ctx_tbl;
	struct ipa_rt_tbl_set rt_tbl_set[IPA_IP_MAX];
	struct ipa_rt_tbl_set reap_rt_tbl_set[IPA_IP_MAX];
	struct kmem_cache *flt_rule_cache;
	struct kmem_cache *rt_rule_cache;
	struct kmem_cache *hdr_cache;
	struct kmem_cache *hdr_offset_cache;
	struct kmem_cache *hdr_proc_ctx_cache;
	struct kmem_cache *hdr_proc_ctx_offset_cache;
	struct kmem_cache *rt_tbl_cache;
	struct kmem_cache *tx_pkt_wrapper_cache;
	struct kmem_cache *rx_pkt_wrapper_cache;
	unsigned long rt_idx_bitmap[IPA_IP_MAX];
	struct mutex lock;
	u16 smem_sz;
	u16 smem_restricted_bytes;
	u16 smem_reqd_sz;
	struct ipa_nat_mem nat_mem;
	u32 excp_hdr_hdl;
	u32 dflt_v4_rt_rule_hdl;
	u32 dflt_v6_rt_rule_hdl;
	uint aggregation_type;
	uint aggregation_byte_limit;
	uint aggregation_time_limit;
	bool hdr_tbl_lcl;
	bool hdr_proc_ctx_tbl_lcl;
	struct ipa_mem_buffer hdr_mem;
	struct ipa_mem_buffer hdr_proc_ctx_mem;
	bool ip4_rt_tbl_lcl;
	bool ip6_rt_tbl_lcl;
	bool ip4_flt_tbl_lcl;
	bool ip6_flt_tbl_lcl;
	struct ipa_mem_buffer empty_rt_tbl_mem;
	struct gen_pool *pipe_mem_pool;
	struct dma_pool *dma_pool;
	struct ipa_active_clients ipa_active_clients;
	struct workqueue_struct *power_mgmt_wq;
	struct workqueue_struct *sps_power_mgmt_wq;
	bool tag_process_before_gating;
	struct ipa_sps_pm sps_pm;
	u32 clnt_hdl_cmd;
	u32 clnt_hdl_data_in;
	u32 clnt_hdl_data_out;
	u8 a5_pipe_index;
	struct list_head intf_list;
	struct list_head msg_list;
	struct list_head pull_msg_list;
	struct mutex msg_lock;
	wait_queue_head_t msg_waitq;
	enum ipa_hw_type ipa_hw_type;
	enum ipa_hw_mode ipa_hw_mode;
	bool use_ipa_teth_bridge;
	
	struct ipa_stats stats;
	void *smem_pipe_mem;
	u32 ipa_bus_hdl;
	struct ipa_controller *ctrl;
	struct idr ipa_idr;
	struct device *pdev;
	spinlock_t idr_lock;
	u32 enable_clock_scaling;
	u32 curr_ipa_clk_rate;
	bool q6_proxy_clk_vote_valid;

	struct ipa_wlan_comm_memb wc_memb;

	struct ipa_uc_ctx uc_ctx;
};

struct ipa_route {
	u32 route_dis;
	u32 route_def_pipe;
	u32 route_def_hdr_table;
	u32 route_def_hdr_ofst;
	u8  route_frag_def_pipe;
};

enum ipa_pipe_mem_type {
	IPA_SPS_PIPE_MEM = 0,
	IPA_PRIVATE_MEM  = 1,
	IPA_SYSTEM_MEM   = 2,
};

struct ipa_plat_drv_res {
	bool use_ipa_teth_bridge;
	u32 ipa_mem_base;
	u32 ipa_mem_size;
	u32 bam_mem_base;
	u32 bam_mem_size;
	u32 ipa_irq;
	u32 bam_irq;
	u32 ipa_pipe_mem_start_ofst;
	u32 ipa_pipe_mem_size;
	enum ipa_hw_type ipa_hw_type;
	enum ipa_hw_mode ipa_hw_mode;
	u32 ee;
};

struct ipa_mem_partition {
	u16 ofst_start;
	u16 nat_ofst;
	u16 nat_size;
	u16 v4_flt_ofst;
	u16 v4_flt_size;
	u16 v4_flt_size_ddr;
	u16 v6_flt_ofst;
	u16 v6_flt_size;
	u16 v6_flt_size_ddr;
	u16 v4_rt_ofst;
	u16 v4_num_index;
	u16 v4_modem_rt_index_lo;
	u16 v4_modem_rt_index_hi;
	u16 v4_apps_rt_index_lo;
	u16 v4_apps_rt_index_hi;
	u16 v4_rt_size;
	u16 v4_rt_size_ddr;
	u16 v6_rt_ofst;
	u16 v6_num_index;
	u16 v6_modem_rt_index_lo;
	u16 v6_modem_rt_index_hi;
	u16 v6_apps_rt_index_lo;
	u16 v6_apps_rt_index_hi;
	u16 v6_rt_size;
	u16 v6_rt_size_ddr;
	u16 modem_hdr_ofst;
	u16 modem_hdr_size;
	u16 apps_hdr_ofst;
	u16 apps_hdr_size;
	u16 apps_hdr_size_ddr;
	u16 modem_hdr_proc_ctx_ofst;
	u16 modem_hdr_proc_ctx_size;
	u16 apps_hdr_proc_ctx_ofst;
	u16 apps_hdr_proc_ctx_size;
	u16 apps_hdr_proc_ctx_size_ddr;
	u16 modem_ofst;
	u16 modem_size;
	u16 apps_v4_flt_ofst;
	u16 apps_v4_flt_size;
	u16 apps_v6_flt_ofst;
	u16 apps_v6_flt_size;
	u16 uc_info_ofst;
	u16 uc_info_size;
	u16 end_ofst;
	u16 apps_v4_rt_ofst;
	u16 apps_v4_rt_size;
	u16 apps_v6_rt_ofst;
	u16 apps_v6_rt_size;
};

struct ipa_controller {
	struct ipa_mem_partition mem_partition;
	u32 ipa_clk_rate_hi;
	u32 ipa_clk_rate_lo;
	u32 clock_scaling_bw_threshold;
	u32 ipa_reg_base_ofst;
	u32 max_holb_tmr_val;
	void (*ipa_sram_read_settings)(void);
	int (*ipa_init_sram)(void);
	int (*ipa_init_hdr)(void);
	int (*ipa_init_rt4)(void);
	int (*ipa_init_rt6)(void);
	int (*ipa_init_flt4)(void);
	int (*ipa_init_flt6)(void);
	void (*ipa_cfg_ep_hdr)(u32 pipe_number,
			const struct ipa_ep_cfg_hdr *ipa_ep_hdr_cfg);
	int (*ipa_cfg_ep_hdr_ext)(u32 pipe_number,
		const struct ipa_ep_cfg_hdr_ext *ipa_ep_hdr_ext_cfg);
	void (*ipa_cfg_ep_aggr)(u32 pipe_number,
			const struct ipa_ep_cfg_aggr *ipa_ep_agrr_cfg);
	int (*ipa_cfg_ep_deaggr)(u32 pipe_index,
			const struct ipa_ep_cfg_deaggr *ep_deaggr);
	void (*ipa_cfg_ep_nat)(u32 pipe_number,
			const struct ipa_ep_cfg_nat *ipa_ep_nat_cfg);
	void (*ipa_cfg_ep_mode)(u32 pipe_number, u32 dst_pipe_number,
			const struct ipa_ep_cfg_mode *ep_mode);
	void (*ipa_cfg_ep_route)(u32 pipe_index, u32 rt_tbl_index);
	void (*ipa_cfg_ep_holb)(u32 pipe_index,
			const struct ipa_ep_cfg_holb *ep_holb);
	void (*ipa_cfg_route)(struct ipa_route *route);
	int (*ipa_read_gen_reg)(char *buff, int max_len);
	int (*ipa_read_ep_reg)(char *buff, int max_len, int pipe);
	void (*ipa_write_dbg_cnt)(int option);
	int (*ipa_read_dbg_cnt)(char *buf, int max_len);
	void (*ipa_cfg_ep_status)(u32 clnt_hdl,
			const struct ipa_ep_cfg_status *ep_status);
	int (*ipa_commit_flt)(enum ipa_ip_type ip);
	int (*ipa_commit_rt)(enum ipa_ip_type ip);
	int (*ipa_generate_rt_hw_rule)(enum ipa_ip_type ip,
		struct ipa_rt_entry *entry, u8 *buf);
	int (*ipa_commit_hdr)(void);
	void (*ipa_cfg_ep_cfg)(u32 clnt_hdl,
			const struct ipa_ep_cfg_cfg *cfg);
	void (*ipa_cfg_ep_metadata_mask)(u32 clnt_hdl,
			const struct ipa_ep_cfg_metadata_mask *metadata_mask);
	void (*ipa_enable_clks)(void);
	void (*ipa_disable_clks)(void);
	struct msm_bus_scale_pdata *msm_bus_data_ptr;

	void (*ipa_cfg_ep_metadata)(u32 pipe_number,
			const struct ipa_ep_cfg_metadata *);
};

extern struct ipa_context *ipa_ctx;

int ipa_send_one(struct ipa_sys_context *sys, struct ipa_desc *desc,
		bool in_atomic);
int ipa_send(struct ipa_sys_context *sys, u32 num_desc, struct ipa_desc *desc,
		bool in_atomic);
int ipa_get_ep_mapping(enum ipa_client_type client);

int ipa_generate_hw_rule(enum ipa_ip_type ip,
			 const struct ipa_rule_attrib *attrib,
			 u8 **buf,
			 u16 *en_rule);
u8 *ipa_write_32(u32 w, u8 *dest);
u8 *ipa_write_16(u16 hw, u8 *dest);
u8 *ipa_write_8(u8 b, u8 *dest);
u8 *ipa_pad_to_32(u8 *dest);
int ipa_init_hw(void);
struct ipa_rt_tbl *__ipa_find_rt_tbl(enum ipa_ip_type ip, const char *name);
int ipa_set_single_ndp_per_mbim(bool);
int ipa_set_hw_timer_fix_for_mbim_aggr(bool);
void ipa_debugfs_init(void);
void ipa_debugfs_remove(void);

void ipa_dump_buff_internal(void *base, dma_addr_t phy_base, u32 size);
#ifdef IPA_DEBUG
#define IPA_DUMP_BUFF(base, phy_base, size) \
	ipa_dump_buff_internal(base, phy_base, size)
#else
#define IPA_DUMP_BUFF(base, phy_base, size)
#endif
int ipa_controller_static_bind(struct ipa_controller *controller,
		enum ipa_hw_type ipa_hw_type);
int ipa_cfg_route(struct ipa_route *route);
int ipa_send_cmd(u16 num_desc, struct ipa_desc *descr);
int ipa_cfg_filter(u32 disable);
int ipa_pipe_mem_init(u32 start_ofst, u32 size);
int ipa_pipe_mem_alloc(u32 *ofst, u32 size);
int ipa_pipe_mem_free(u32 ofst, u32 size);
int ipa_straddle_boundary(u32 start, u32 end, u32 boundary);
struct ipa_context *ipa_get_ctx(void);
void ipa_enable_clks(void);
void ipa_disable_clks(void);
void ipa_inc_client_enable_clks(void);
int ipa_inc_client_enable_clks_no_block(void);
void ipa_dec_client_disable_clks(void);
int ipa_interrupts_init(u32 ipa_irq, u32 ee, struct device *ipa_dev);
int __ipa_del_rt_rule(u32 rule_hdl);
int __ipa_del_hdr(u32 hdr_hdl);
int __ipa_release_hdr(u32 hdr_hdl);
int __ipa_release_hdr_proc_ctx(u32 proc_ctx_hdl);
int _ipa_read_gen_reg_v1_0(char *buff, int max_len);
int _ipa_read_gen_reg_v1_1(char *buff, int max_len);
int _ipa_read_gen_reg_v2_0(char *buff, int max_len);
int _ipa_read_ep_reg_v1_0(char *buf, int max_len, int pipe);
int _ipa_read_ep_reg_v1_1(char *buf, int max_len, int pipe);
int _ipa_read_ep_reg_v2_0(char *buf, int max_len, int pipe);
void _ipa_write_dbg_cnt_v1(int option);
void _ipa_write_dbg_cnt_v2_0(int option);
int _ipa_read_dbg_cnt_v1(char *buf, int max_len);
int _ipa_read_dbg_cnt_v2_0(char *buf, int max_len);
void _ipa_enable_clks_v1(void);
void _ipa_enable_clks_v2_0(void);
void _ipa_disable_clks_v1(void);
void _ipa_disable_clks_v2_0(void);

static inline u32 ipa_read_reg(void *base, u32 offset)
{
	return ioread32(base + offset);
}

static inline u32 ipa_read_reg_field(void *base, u32 offset,
		u32 mask, u32 shift)
{
	return (ipa_read_reg(base, offset) & mask) >> shift;
}

static inline void ipa_write_reg(void *base, u32 offset, u32 val)
{
	iowrite32(val, base + offset);
}

int ipa_bridge_init(void);
void ipa_bridge_cleanup(void);

ssize_t ipa_read(struct file *filp, char __user *buf, size_t count,
		 loff_t *f_pos);
int ipa_pull_msg(struct ipa_msg_meta *meta, char *buff, size_t count);
int ipa_query_intf(struct ipa_ioc_query_intf *lookup);
int ipa_query_intf_tx_props(struct ipa_ioc_query_intf_tx_props *tx);
int ipa_query_intf_rx_props(struct ipa_ioc_query_intf_rx_props *rx);
int ipa_query_intf_ext_props(struct ipa_ioc_query_intf_ext_props *ext);

void wwan_cleanup(void);

int teth_bridge_driver_init(void);
void ipa_lan_rx_cb(void *priv, enum ipa_dp_evt_type evt, unsigned long data);

int _ipa_init_sram_v2(void);
int _ipa_init_sram_v2_5(void);
int _ipa_init_hdr_v2(void);
int _ipa_init_hdr_v2_5(void);
int _ipa_init_rt4_v2(void);
int _ipa_init_rt6_v2(void);
int _ipa_init_flt4_v2(void);
int _ipa_init_flt6_v2(void);

int __ipa_commit_flt_v1(enum ipa_ip_type ip);
int __ipa_commit_flt_v2(enum ipa_ip_type ip);
int __ipa_commit_rt_v1(enum ipa_ip_type ip);
int __ipa_commit_rt_v2(enum ipa_ip_type ip);
int __ipa_generate_rt_hw_rule_v2(enum ipa_ip_type ip,
	struct ipa_rt_entry *entry, u8 *buf);
int __ipa_generate_rt_hw_rule_v2_5(enum ipa_ip_type ip,
	struct ipa_rt_entry *entry, u8 *buf);

int __ipa_commit_hdr_v1(void);
int __ipa_commit_hdr_v2(void);
int __ipa_commit_hdr_v2_5(void);
int ipa_generate_flt_eq(enum ipa_ip_type ip,
		const struct ipa_rule_attrib *attrib,
		struct ipa_ipfltri_rule_eq *eq_attrib);
void ipa_skb_recycle(struct sk_buff *skb);
void ipa_install_dflt_flt_rules(u32 ipa_ep_idx);
void ipa_delete_dflt_flt_rules(u32 ipa_ep_idx);

int ipa_enable_data_path(u32 clnt_hdl);
int ipa_disable_data_path(u32 clnt_hdl);
int ipa_id_alloc(void *ptr);
void *ipa_id_find(u32 id);
void ipa_id_remove(u32 id);

int ipa_set_required_perf_profile(enum ipa_voltage_level floor_voltage,
				  u32 bandwidth_mbps);

int ipa_cfg_ep_status(u32 clnt_hdl, const struct ipa_ep_cfg_status *ipa_ep_cfg);
int ipa_cfg_aggr_cntr_granularity(u8 aggr_granularity);
int ipa_cfg_eot_coal_cntr_granularity(u8 eot_coal_granularity);

int ipa_suspend_resource_no_block(enum ipa_rm_resource_name name);
int ipa_suspend_resource_sync(enum ipa_rm_resource_name name);
int ipa_resume_resource(enum ipa_rm_resource_name name);
bool ipa_should_pipe_be_suspended(enum ipa_client_type client);
int ipa_tag_aggr_force_close(int pipe_num);

void ipa_active_clients_lock(void);
int ipa_active_clients_trylock(unsigned long *flags);
void ipa_active_clients_unlock(void);
void ipa_active_clients_trylock_unlock(unsigned long *flags);
int ipa_wdi_init(void);
int ipa_write_qmapid_wdi_pipe(u32 clnt_hdl, u8 qmap_id);
int ipa_tag_process(struct ipa_desc *desc, int num_descs,
		    unsigned long timeout);

int ipa_q6_cleanup(void);
int ipa_init_q6_smem(void);

int ipa_sps_connect_safe(struct sps_pipe *h, struct sps_connect *connect,
			 enum ipa_client_type ipa_client);
int ipa_uc_interface_init(void);
int ipa_uc_reset_pipe(enum ipa_client_type ipa_client);
void ipa_register_panic_hdlr(void);
int create_nat_device(void);
int ipa_uc_notify_clk_state(bool enabled);
#endif 
