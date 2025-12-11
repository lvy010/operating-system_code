/* SPDX-License-Identifier: GPL-2.0 */
#ifndef BLK_MQ_H
#define BLK_MQ_H

#include <linux/blkdev.h>
#include <linux/sbitmap.h>
#include <linux/lockdep.h>
#include <linux/scatterlist.h>
#include <linux/prefetch.h>
#include <linux/srcu.h>
#include <linux/rw_hint.h>

struct blk_mq_tags;
struct blk_flush_queue;

/* 定义每个块设备最少和默认的请求队列大小 */
#define BLKDEV_MIN_RQ	4
#define BLKDEV_DEFAULT_RQ	128

/* 请求结束时的返回值枚举 */
enum rq_end_io_ret {
	RQ_END_IO_NONE,    /* 请求未完成 */
	RQ_END_IO_FREE,    /* 请求已完成,可以释放 */
};

/* 请求结束时的回调函数类型定义 */
typedef enum rq_end_io_ret (rq_end_io_fn)(struct request *, blk_status_t);

/*
 * 请求标志位定义
 * 这些标志位用于标记请求的各种状态和特性
 */
typedef __u32 __bitwise req_flags_t;

/* 请求标志位的具体定义 */
enum rqf_flags {
	__RQF_STARTED,           /* 请求已经开始执行 */
	__RQF_FLUSH_SEQ,         /* 这是一个刷新序列请求 */
	__RQF_MIXED_MERGE,       /* 不同类型的请求合并 */
	__RQF_DONTPREP,          /* 不需要预处理 */
	__RQF_SCHED_TAGS,        /* 使用调度器标签 */
	__RQF_USE_SCHED,         /* 使用IO调度器 */
	__RQF_FAILED,            /* 请求执行失败 */
	__RQF_QUIET,             /* 错误时不发出警告 */
	__RQF_IO_STAT,           /* 计入IO统计 */
	__RQF_PM,                /* 电源管理相关请求 */
	__RQF_HASHED,            /* 在调度器合并哈希表中 */
	__RQF_STATS,             /* 跟踪IO完成时间 */
	__RQF_SPECIAL_PAYLOAD,   /* 使用特殊数据负载 */
	__RQF_ZONE_WRITE_PLUGGING, /* 需要通知区域写入插拔 */
	__RQF_TIMED_OUT,         /* 请求已超时 */
	__RQF_RESV,              /* 保留标志位 */
	__RQF_BITS               /* 标志位总数 */
};

#define RQF_STARTED		((__force req_flags_t)(1 << __RQF_STARTED))
#define RQF_FLUSH_SEQ		((__force req_flags_t)(1 << __RQF_FLUSH_SEQ))
#define RQF_MIXED_MERGE		((__force req_flags_t)(1 << __RQF_MIXED_MERGE))
#define RQF_DONTPREP		((__force req_flags_t)(1 << __RQF_DONTPREP))
#define RQF_SCHED_TAGS		((__force req_flags_t)(1 << __RQF_SCHED_TAGS))
#define RQF_USE_SCHED		((__force req_flags_t)(1 << __RQF_USE_SCHED))
#define RQF_FAILED		((__force req_flags_t)(1 << __RQF_FAILED))
#define RQF_QUIET		((__force req_flags_t)(1 << __RQF_QUIET))
#define RQF_IO_STAT		((__force req_flags_t)(1 << __RQF_IO_STAT))
#define RQF_PM			((__force req_flags_t)(1 << __RQF_PM))
#define RQF_HASHED		((__force req_flags_t)(1 << __RQF_HASHED))
#define RQF_STATS		((__force req_flags_t)(1 << __RQF_STATS))
#define RQF_SPECIAL_PAYLOAD	\
			((__force req_flags_t)(1 << __RQF_SPECIAL_PAYLOAD))
#define RQF_ZONE_WRITE_PLUGGING	\
			((__force req_flags_t)(1 << __RQF_ZONE_WRITE_PLUGGING))
#define RQF_TIMED_OUT		((__force req_flags_t)(1 << __RQF_TIMED_OUT))
#define RQF_RESV		((__force req_flags_t)(1 << __RQF_RESV))

/* flags that prevent us from merging requests: */
#define RQF_NOMERGE_FLAGS \
	(RQF_STARTED | RQF_FLUSH_SEQ | RQF_SPECIAL_PAYLOAD)

/* 请求状态枚举 */
enum mq_rq_state {
	MQ_RQ_IDLE		= 0,    /* 请求空闲 */
	MQ_RQ_IN_FLIGHT		= 1,    /* 请求正在执行 */
	MQ_RQ_COMPLETE		= 2,    /* 请求已完成 */
};

/*
 * Try to put the fields that are referenced together in the same cacheline.
 *
 * If you modify this structure, make sure to update blk_rq_init() and
 * especially blk_mq_rq_ctx_init() to take care of the added fields.
 */
struct request {
	struct request_queue *q;          /* 所属的请求队列 */
	struct blk_mq_ctx *mq_ctx;        /* 多队列上下文 */
	struct blk_mq_hw_ctx *mq_hctx;    /* 硬件队列上下文 */

	blk_opf_t cmd_flags;              /* 操作类型和通用标志 */
	req_flags_t rq_flags;             /* 请求特定标志 */

	int tag;                          /* 请求标签 */
	int internal_tag;                 /* 内部标签 */

	unsigned int timeout;             /* 超时时间 */

	/* 以下两个字段是内部使用的,不要直接访问 */
	unsigned int __data_len;          /* 总数据长度 */
	sector_t __sector;                /* 扇区位置 */

	struct bio *bio;                  /* 第一个bio结构 */
	struct bio *biotail;              /* 最后一个bio结构 */

	union {
		struct list_head queuelist;   /* 队列链表 */
		struct request *rq_next;      /* 下一个请求 */
	};

	struct block_device *part;
#ifdef CONFIG_BLK_RQ_ALLOC_TIME
	/* Time that the first bio started allocating this request. */
	u64 alloc_time_ns;
#endif
	/* Time that this request was allocated for this IO. */
	u64 start_time_ns;
	/* Time that I/O was submitted to the device. */
	u64 io_start_time_ns;

#ifdef CONFIG_BLK_WBT
	unsigned short wbt_flags;
#endif
	/*
	 * rq sectors used for blk stats. It has the same value
	 * with blk_rq_sectors(rq), except that it never be zeroed
	 * by completion.
	 */
	unsigned short stats_sectors;

	/*
	 * Number of scatter-gather DMA addr+len pairs after
	 * physical address coalescing is performed.
	 */
	unsigned short nr_phys_segments;
	unsigned short nr_integrity_segments;

#ifdef CONFIG_BLK_INLINE_ENCRYPTION
	struct bio_crypt_ctx *crypt_ctx;
	struct blk_crypto_keyslot *crypt_keyslot;
#endif

	enum mq_rq_state state;
	atomic_t ref;

	unsigned long deadline;

	/*
	 * The hash is used inside the scheduler, and killed once the
	 * request reaches the dispatch list. The ipi_list is only used
	 * to queue the request for softirq completion, which is long
	 * after the request has been unhashed (and even removed from
	 * the dispatch list).
	 */
	union {
		struct hlist_node hash;	/* merge hash */
		struct llist_node ipi_list;
	};

	/*
	 * The rb_node is only used inside the io scheduler, requests
	 * are pruned when moved to the dispatch queue. special_vec must
	 * only be used if RQF_SPECIAL_PAYLOAD is set, and those cannot be
	 * insert into an IO scheduler.
	 */
	union {
		struct rb_node rb_node;	/* sort/lookup */
		struct bio_vec special_vec;
	};

	/*
	 * Three pointers are available for the IO schedulers, if they need
	 * more they have to dynamically allocate it.
	 */
	struct {
		struct io_cq		*icq;
		void			*priv[2];
	} elv;

	struct {
		unsigned int		seq;
		rq_end_io_fn		*saved_end_io;
	} flush;

	u64 fifo_time;

	/*
	 * completion callback.
	 */
	rq_end_io_fn *end_io;
	void *end_io_data;
};

static inline enum req_op req_op(const struct request *req)
{
	return req->cmd_flags & REQ_OP_MASK;
}

static inline bool blk_rq_is_passthrough(struct request *rq)
{
	return blk_op_is_passthrough(rq->cmd_flags);
}

static inline unsigned short req_get_ioprio(struct request *req)
{
	if (req->bio)
		return req->bio->bi_ioprio;
	return 0;
}

#define rq_data_dir(rq)		(op_is_write(req_op(rq)) ? WRITE : READ)

#define rq_dma_dir(rq) \
	(op_is_write(req_op(rq)) ? DMA_TO_DEVICE : DMA_FROM_DEVICE)

static inline int rq_list_empty(const struct rq_list *rl)
{
	return rl->head == NULL;
}

static inline void rq_list_init(struct rq_list *rl)
{
	rl->head = NULL;
	rl->tail = NULL;
}

static inline void rq_list_add_tail(struct rq_list *rl, struct request *rq)
{
	rq->rq_next = NULL;
	if (rl->tail)
		rl->tail->rq_next = rq;
	else
		rl->head = rq;
	rl->tail = rq;
}

static inline void rq_list_add_head(struct rq_list *rl, struct request *rq)
{
	rq->rq_next = rl->head;
	rl->head = rq;
	if (!rl->tail)
		rl->tail = rq;
}

static inline struct request *rq_list_pop(struct rq_list *rl)
{
	struct request *rq = rl->head;

	if (rq) {
		rl->head = rl->head->rq_next;
		if (!rl->head)
			rl->tail = NULL;
		rq->rq_next = NULL;
	}

	return rq;
}

static inline struct request *rq_list_peek(struct rq_list *rl)
{
	return rl->head;
}

#define rq_list_for_each(rl, pos)					\
	for (pos = rq_list_peek((rl)); (pos); pos = pos->rq_next)

#define rq_list_for_each_safe(rl, pos, nxt)				\
	for (pos = rq_list_peek((rl)), nxt = pos->rq_next;		\
		pos; pos = nxt, nxt = pos ? pos->rq_next : NULL)

/**
 * enum blk_eh_timer_return - How the timeout handler should proceed
 * @BLK_EH_DONE: The block driver completed the command or will complete it at
 *	a later time.
 * @BLK_EH_RESET_TIMER: Reset the request timer and continue waiting for the
 *	request to complete.
 */
enum blk_eh_timer_return {
	BLK_EH_DONE,
	BLK_EH_RESET_TIMER,
};

/**
 * struct blk_mq_hw_ctx - State for a hardware queue facing the hardware
 * block device
 */
struct blk_mq_hw_ctx {
	struct {
		/** @lock: Protects the dispatch list. */
		spinlock_t		lock;      /* 保护dispatch列表的锁 */
		/**
		 * @dispatch: Used for requests that are ready to be
		 * dispatched to the hardware but for some reason (e.g. lack of
		 * resources) could not be sent to the hardware. As soon as the
		 * driver can send new requests, requests at this list will
		 * be sent first for a fairer dispatch.
		 */
		struct list_head	dispatch;  /* 待分发的请求列表 */
		 /**
		  * @state: BLK_MQ_S_* flags. Defines the state of the hw
		  * queue (active, scheduled to restart, stopped).
		  */
		unsigned long		state;     /* 硬件队列状态 */
	} ____cacheline_aligned_in_smp;

	/**
	 * @run_work: Used for scheduling a hardware queue run at a later time.
	 */
	struct delayed_work	run_work;    /* 延迟运行工作 */
	/** @cpumask: Map of available CPUs where this hctx can run. */
	cpumask_var_t		cpumask;     /* 可用的CPU掩码 */
	/**
	 * @next_cpu: Used by blk_mq_hctx_next_cpu() for round-robin CPU
	 * selection from @cpumask.
	 */
	int			next_cpu;    /* 下一个要使用的CPU */
	/**
	 * @next_cpu_batch: Counter of how many works left in the batch before
	 * changing to the next CPU.
	 */
	int			next_cpu_batch; /* CPU批处理计数 */

	/** @flags: BLK_MQ_F_* flags. Defines the behaviour of the queue. */
	unsigned long		flags;

	/**
	 * @sched_data: Pointer owned by the IO scheduler attached to a request
	 * queue. It's up to the IO scheduler how to use this pointer.
	 */
	void			*sched_data;
	/**
	 * @queue: Pointer to the request queue that owns this hardware context.
	 */
	struct request_queue	*queue;
	/** @fq: Queue of requests that need to perform a flush operation. */
	struct blk_flush_queue	*fq;

	/**
	 * @driver_data: Pointer to data owned by the block driver that created
	 * this hctx
	 */
	void			*driver_data;

	/**
	 * @ctx_map: Bitmap for each software queue. If bit is on, there is a
	 * pending request in that software queue.
	 */
	struct sbitmap		ctx_map;

	/**
	 * @dispatch_from: Software queue to be used when no scheduler was
	 * selected.
	 */
	struct blk_mq_ctx	*dispatch_from;
	/**
	 * @dispatch_busy: Number used by blk_mq_update_dispatch_busy() to
	 * decide if the hw_queue is busy using Exponential Weighted Moving
	 * Average algorithm.
	 */
	unsigned int		dispatch_busy;

	/** @type: HCTX_TYPE_* flags. Type of hardware queue. */
	unsigned short		type;
	/** @nr_ctx: Number of software queues. */
	unsigned short		nr_ctx;
	/** @ctxs: Array of software queues. */
	struct blk_mq_ctx	**ctxs;

	/** @dispatch_wait_lock: Lock for dispatch_wait queue. */
	spinlock_t		dispatch_wait_lock;
	/**
	 * @dispatch_wait: Waitqueue to put requests when there is no tag
	 * available at the moment, to wait for another try in the future.
	 */
	wait_queue_entry_t	dispatch_wait;

	/**
	 * @wait_index: Index of next available dispatch_wait queue to insert
	 * requests.
	 */
	atomic_t		wait_index;

	/**
	 * @tags: Tags owned by the block driver. A tag at this set is only
	 * assigned when a request is dispatched from a hardware queue.
	 */
	struct blk_mq_tags	*tags;
	/**
	 * @sched_tags: Tags owned by I/O scheduler. If there is an I/O
	 * scheduler associated with a request queue, a tag is assigned when
	 * that request is allocated. Else, this member is not used.
	 */
	struct blk_mq_tags	*sched_tags;

	/** @numa_node: NUMA node the storage adapter has been connected to. */
	unsigned int		numa_node;
	/** @queue_num: Index of this hardware queue. */
	unsigned int		queue_num;

	/**
	 * @nr_active: Number of active requests. Only used when a tag set is
	 * shared across request queues.
	 */
	atomic_t		nr_active;

	/** @cpuhp_online: List to store request if CPU is going to die */
	struct hlist_node	cpuhp_online;
	/** @cpuhp_dead: List to store request if some CPU die. */
	struct hlist_node	cpuhp_dead;
	/** @kobj: Kernel object for sysfs. */
	struct kobject		kobj;

#ifdef CONFIG_BLK_DEBUG_FS
	/**
	 * @debugfs_dir: debugfs directory for this hardware queue. Named
	 * as cpu<cpu_number>.
	 */
	struct dentry		*debugfs_dir;
	/** @sched_debugfs_dir:	debugfs directory for the scheduler. */
	struct dentry		*sched_debugfs_dir;
#endif

	/**
	 * @hctx_list: if this hctx is not in use, this is an entry in
	 * q->unused_hctx_list.
	 */
	struct list_head	hctx_list;
};

/**
 * struct blk_mq_queue_map - Map software queues to hardware queues
 * @mq_map:       CPU ID to hardware queue index map. This is an array
 *	with nr_cpu_ids elements. Each element has a value in the range
 *	[@queue_offset, @queue_offset + @nr_queues).
 * @nr_queues:    Number of hardware queues to map CPU IDs onto.
 * @queue_offset: First hardware queue to map onto. Used by the PCIe NVMe
 *	driver to map each hardware queue type (enum hctx_type) onto a distinct
 *	set of hardware queues.
 */
struct blk_mq_queue_map {
	unsigned int *mq_map;
	unsigned int nr_queues;
	unsigned int queue_offset;
};

/* 硬件队列类型枚举 */
enum hctx_type {
	HCTX_TYPE_DEFAULT,    /* 默认IO类型 */
	HCTX_TYPE_READ,       /* 读IO类型 */
	HCTX_TYPE_POLL,       /* 轮询IO类型 */
	HCTX_MAX_TYPES,       /* 类型总数 */
};

/* 标签集结构体
 * 用于管理请求队列的标签分配
 */
struct blk_mq_tag_set {
	const struct blk_mq_ops	*ops;           /* 操作函数集 */
	struct blk_mq_queue_map	map[HCTX_MAX_TYPES];  /* 队列映射 */
	unsigned int		nr_maps;        /* 映射数量 */
	unsigned int		nr_hw_queues;   /* 硬件队列数量 */
	unsigned int		queue_depth;    /* 队列深度 */
	unsigned int		reserved_tags;  /* 保留标签数 */
	unsigned int		cmd_size;       /* 命令大小 */
	int			numa_node;      /* NUMA节点 */
	unsigned int		timeout;        /* 超时时间 */
	unsigned int		flags;          /* 标志位 */
	void			*driver_data;   /* 驱动私有数据 */

	struct blk_mq_tags	**tags;         /* 标签数组 */
	struct blk_mq_tags	*shared_tags;   /* 共享标签 */

	struct mutex		tag_list_lock;  /* 标签列表锁 */
	struct list_head	tag_list;       /* 标签列表 */
	struct srcu_struct	*srcu;          /* SRCU锁 */
};

/* 队列数据结构体
 * 用于传递请求到队列时的数据
 */
struct blk_mq_queue_data {
	struct request *rq;    /* 请求指针 */
	bool last;             /* 是否是最后一个请求 */
};

/* 标签迭代函数类型定义 */
typedef bool (busy_tag_iter_fn)(struct request *, void *);

/* 多队列操作函数集
 * 定义了块设备驱动需要实现的操作函数
 */
struct blk_mq_ops {
	/* 将新请求加入队列 */
	blk_status_t (*queue_rq)(struct blk_mq_hw_ctx *,
				 const struct blk_mq_queue_data *);

	/* 提交请求到硬件 */
	void (*commit_rqs)(struct blk_mq_hw_ctx *);

	/* 批量提交请求 */
	void (*queue_rqs)(struct rq_list *rqlist);

	/* 获取预算 */
	int (*get_budget)(struct request_queue *);

	/* 释放预算 */
	void (*put_budget)(struct request_queue *, int);

	/* 设置请求预算令牌 */
	void (*set_rq_budget_token)(struct request *, int);
	
	/* 获取请求预算令牌 */
	int (*get_rq_budget_token)(struct request *);

	/* 请求超时处理 */
	enum blk_eh_timer_return (*timeout)(struct request *);

	/* 轮询请求完成状态 */
	int (*poll)(struct blk_mq_hw_ctx *, struct io_comp_batch *);

	/* 标记请求完成 */
	void (*complete)(struct request *);

	/* 初始化硬件队列上下文 */
	int (*init_hctx)(struct blk_mq_hw_ctx *, void *, unsigned int);
	
	/* 清理硬件队列上下文 */
	void (*exit_hctx)(struct blk_mq_hw_ctx *, unsigned int);

	/* 初始化请求 */
	int (*init_request)(struct blk_mq_tag_set *set, struct request *,
			    unsigned int, unsigned int);
	
	/* 清理请求 */
	void (*exit_request)(struct blk_mq_tag_set *set, struct request *,
			     unsigned int);

	/* 清理未完成的请求 */
	void (*cleanup_rq)(struct request *);

	/* 检查队列是否忙 */
	bool (*busy)(struct request_queue *);

	/* 映射队列 */
	void (*map_queues)(struct blk_mq_tag_set *set);

#ifdef CONFIG_BLK_DEBUG_FS
	/* 显示请求的调试信息 */
	void (*show_rq)(struct seq_file *m, struct request *rq);
#endif
};

/* 多队列标志位定义 */
enum {
	BLK_MQ_F_TAG_QUEUE_SHARED = 1 << 1,    /* 标签队列共享 */
	BLK_MQ_F_STACKING	= 1 << 2,        /* 需要底层blk-mq设备完成IO */
	BLK_MQ_F_TAG_HCTX_SHARED = 1 << 3,     /* 硬件上下文共享标签 */
	BLK_MQ_F_BLOCKING	= 1 << 4,        /* 阻塞式队列 */
	BLK_MQ_F_TAG_RR		= 1 << 5,        /* 轮询分配标签 */
	BLK_MQ_F_NO_SCHED_BY_DEFAULT = 1 << 6, /* 默认不使用调度器 */
	BLK_MQ_F_MAX = 1 << 7,                 /* 最大标志位 */
};

/* 最大队列深度 */
#define BLK_MQ_MAX_DEPTH	(10240)
/* 无效的硬件上下文索引 */
#define BLK_MQ_NO_HCTX_IDX	(-1U)

/* 硬件队列状态定义 */
enum {
	BLK_MQ_S_STOPPED,           /* 队列已停止 */
	BLK_MQ_S_TAG_ACTIVE,        /* 标签活跃 */
	BLK_MQ_S_SCHED_RESTART,     /* 调度器重启 */
	BLK_MQ_S_INACTIVE,          /* 队列不活跃(所有CPU离线) */
	BLK_MQ_S_MAX                /* 状态总数 */
};

/* 请求分配标志位 */
enum {
	BLK_MQ_REQ_NOWAIT	= (__force blk_mq_req_flags_t)(1 << 0),  /* 无等待分配 */
	BLK_MQ_REQ_RESERVED	= (__force blk_mq_req_flags_t)(1 << 1),  /* 从保留池分配 */
	BLK_MQ_REQ_PM		= (__force blk_mq_req_flags_t)(1 << 2),  /* 电源管理请求 */
};

/* 标签结构体
 * 用于管理请求的标签分配和跟踪
 */
struct blk_mq_tags {
	unsigned int nr_tags;              /* 标签总数 */
	unsigned int nr_reserved_tags;     /* 保留标签数 */
	unsigned int active_queues;        /* 活跃队列数 */

	struct sbitmap_queue bitmap_tags;  /* 位图标签 */
	struct sbitmap_queue breserved_tags; /* 保留位图标签 */

	struct request **rqs;              /* 请求指针数组 */
	struct request **static_rqs;       /* 静态请求数组 */
	struct list_head page_list;        /* 页面列表 */

	spinlock_t lock;                   /* 保护标签的锁 */
};

/* 根据标签获取请求 */
static inline struct request *blk_mq_tag_to_rq(struct blk_mq_tags *tags,
					       unsigned int tag)
{
	if (tag < tags->nr_tags) {
		prefetch(tags->rqs[tag]);
		return tags->rqs[tag];
	}

	return NULL;
}

/* 唯一标签相关定义 */
enum {
	BLK_MQ_UNIQUE_TAG_BITS = 16,                    /* 唯一标签位数 */
	BLK_MQ_UNIQUE_TAG_MASK = (1 << BLK_MQ_UNIQUE_TAG_BITS) - 1,  /* 唯一标签掩码 */
};

/* 获取请求的唯一标签 */
u32 blk_mq_unique_tag(struct request *rq);

/* 从唯一标签获取硬件队列索引 */
static inline u16 blk_mq_unique_tag_to_hwq(u32 unique_tag)
{
	return unique_tag >> BLK_MQ_UNIQUE_TAG_BITS;
}

/* 从唯一标签获取标签值 */
static inline u16 blk_mq_unique_tag_to_tag(u32 unique_tag)
{
	return unique_tag & BLK_MQ_UNIQUE_TAG_MASK;
}

/* 获取请求的当前状态 */
static inline enum mq_rq_state blk_mq_rq_state(struct request *rq)
{
	return READ_ONCE(rq->state);
}

/* 检查请求是否已开始 */
static inline int blk_mq_request_started(struct request *rq)
{
	return blk_mq_rq_state(rq) != MQ_RQ_IDLE;
}

/* 检查请求是否已完成 */
static inline int blk_mq_request_completed(struct request *rq)
{
	return blk_mq_rq_state(rq) == MQ_RQ_COMPLETE;
}

/* 设置请求为完成状态 */
static inline void blk_mq_set_request_complete(struct request *rq)
{
	WRITE_ONCE(rq->state, MQ_RQ_COMPLETE);
}

/* 直接完成请求
 * 用于在可抢占环境中直接完成请求,而不是通过软中断或另一个CPU
 */
static inline void blk_mq_complete_request_direct(struct request *rq,
		   void (*complete)(struct request *rq))
{
	WRITE_ONCE(rq->state, MQ_RQ_COMPLETE);
	complete(rq);
}

/* 开始处理请求 */
void blk_mq_start_request(struct request *rq);

/* 结束请求处理 */
void blk_mq_end_request(struct request *rq, blk_status_t error);
void __blk_mq_end_request(struct request *rq, blk_status_t error);

/* 批量结束请求 */
void blk_mq_end_request_batch(struct io_comp_batch *ib);

/* 检查是否需要时间戳
 * 当启用了IO统计、块统计或使用IO调度器时需要
 */
static inline bool blk_mq_need_time_stamp(struct request *rq)
{
	return (rq->rq_flags & (RQF_IO_STAT | RQF_STATS | RQF_USE_SCHED));
}

/* 检查是否是保留请求 */
static inline bool blk_mq_is_reserved_rq(struct request *rq)
{
	return rq->rq_flags & RQF_RESV;
}

/* 将请求添加到完成批处理中
 * @req: 要添加的请求
 * @iob: 批处理容器
 * @is_error: 请求是否失败
 * @complete: 请求的完成处理函数
 * 
 * 批处理完成只在没有IO错误且没有特殊的end_io处理函数时工作
 * 
 * 返回值: 请求是否成功添加到批处理中
 */
static inline bool blk_mq_add_to_batch(struct request *req,
				       struct io_comp_batch *iob, bool is_error,
				       void (*complete)(struct io_comp_batch *))
{
	/* 检查各种排除批处理的条件 */
	if (!iob)
		return false;
	if (req->rq_flags & RQF_SCHED_TAGS)
		return false;
	if (!blk_rq_is_passthrough(req)) {
		if (req->end_io)
			return false;
		if (is_error)
			return false;
	}

	if (!iob->complete)
		iob->complete = complete;
	else if (iob->complete != complete)
		return false;
	iob->need_ts |= blk_mq_need_time_stamp(req);
	rq_list_add_tail(&iob->req_list, req);
	return true;
}

/* 重新排队请求 */
void blk_mq_requeue_request(struct request *rq, bool kick_requeue_list);

/* 触发重新排队列表处理 */
void blk_mq_kick_requeue_list(struct request_queue *q);

/* 延迟触发重新排队列表处理 */
void blk_mq_delay_kick_requeue_list(struct request_queue *q, unsigned long msecs);

/* 完成请求处理 */
void blk_mq_complete_request(struct request *rq);

/* 远程完成请求 */
bool blk_mq_complete_request_remote(struct request *rq);

/* 停止硬件队列 */
void blk_mq_stop_hw_queue(struct blk_mq_hw_ctx *hctx);

/* 启动硬件队列 */
void blk_mq_start_hw_queue(struct blk_mq_hw_ctx *hctx);

/* 停止所有硬件队列 */
void blk_mq_stop_hw_queues(struct request_queue *q);

/* 启动所有硬件队列 */
void blk_mq_start_hw_queues(struct request_queue *q);

/* 启动已停止的硬件队列 */
void blk_mq_start_stopped_hw_queue(struct blk_mq_hw_ctx *hctx, bool async);

/* 启动所有已停止的硬件队列 */
void blk_mq_start_stopped_hw_queues(struct request_queue *q, bool async);

/* 暂停队列处理 */
void blk_mq_quiesce_queue(struct request_queue *q);

/* 等待队列暂停完成 */
void blk_mq_wait_quiesce_done(struct blk_mq_tag_set *set);

/* 暂停标签集 */
void blk_mq_quiesce_tagset(struct blk_mq_tag_set *set);

/* 恢复标签集 */
void blk_mq_unquiesce_tagset(struct blk_mq_tag_set *set);

/* 恢复队列处理 */
void blk_mq_unquiesce_queue(struct request_queue *q);

/* 延迟运行硬件队列 */
void blk_mq_delay_run_hw_queue(struct blk_mq_hw_ctx *hctx, unsigned long msecs);

/* 运行硬件队列 */
void blk_mq_run_hw_queue(struct blk_mq_hw_ctx *hctx, bool async);

/* 运行所有硬件队列 */
void blk_mq_run_hw_queues(struct request_queue *q, bool async);

/* 延迟运行所有硬件队列 */
void blk_mq_delay_run_hw_queues(struct request_queue *q, unsigned long msecs);

/* 遍历标签集中的忙标签 */
void blk_mq_tagset_busy_iter(struct blk_mq_tag_set *tagset,
		busy_tag_iter_fn *fn, void *priv);

/* 等待标签集中的请求完成 */
void blk_mq_tagset_wait_completed_request(struct blk_mq_tag_set *tagset);

/* 冻结队列(不保存内存状态) */
void blk_mq_freeze_queue_nomemsave(struct request_queue *q);

/* 解冻队列(不恢复内存状态) */
void blk_mq_unfreeze_queue_nomemrestore(struct request_queue *q);

/* 冻结队列并保存内存状态 */
static inline unsigned int __must_check
blk_mq_freeze_queue(struct request_queue *q)
{
	unsigned int memflags = memalloc_noio_save();

	blk_mq_freeze_queue_nomemsave(q);
	return memflags;
}

/* 解冻队列并恢复内存状态 */
static inline void
blk_mq_unfreeze_queue(struct request_queue *q, unsigned int memflags)
{
	blk_mq_unfreeze_queue_nomemrestore(q);
	memalloc_noio_restore(memflags);
}

/* 开始冻结队列 */
void blk_freeze_queue_start(struct request_queue *q);

/* 等待队列冻结完成 */
void blk_mq_freeze_queue_wait(struct request_queue *q);

/* 带超时的等待队列冻结完成 */
int blk_mq_freeze_queue_wait_timeout(struct request_queue *q,
				     unsigned long timeout);

/* 非所有者解冻队列 */
void blk_mq_unfreeze_queue_non_owner(struct request_queue *q);

/* 非所有者开始冻结队列 */
void blk_freeze_queue_start_non_owner(struct request_queue *q);

/* 映射队列 */
void blk_mq_map_queues(struct blk_mq_queue_map *qmap);

/* 映射硬件队列 */
void blk_mq_map_hw_queues(struct blk_mq_queue_map *qmap,
			  struct device *dev, unsigned int offset);

/* 更新硬件队列数量 */
void blk_mq_update_nr_hw_queues(struct blk_mq_tag_set *set, int nr_hw_queues);

/* 立即暂停队列 */
void blk_mq_quiesce_queue_nowait(struct request_queue *q);

/* 获取请求的CPU */
unsigned int blk_mq_rq_cpu(struct request *rq);

/* 检查是否应该模拟超时 */
bool __blk_should_fake_timeout(struct request_queue *q);
static inline bool blk_should_fake_timeout(struct request_queue *q)
{
	if (IS_ENABLED(CONFIG_FAIL_IO_TIMEOUT) &&
	    test_bit(QUEUE_FLAG_FAIL_IO, &q->queue_flags))
		return __blk_should_fake_timeout(q);
	return false;
}

/* 从PDU获取请求
 * @pdu: 协议数据单元
 * 
 * 返回值: 请求结构体指针
 * 
 * 驱动命令数据紧跟在请求结构体之后,所以减去请求大小即可得到原始请求
 */
static inline struct request *blk_mq_rq_from_pdu(void *pdu)
{
	return pdu - sizeof(struct request);
}

/* 从请求获取PDU
 * @rq: 请求结构体
 * 
 * 返回值: PDU指针
 * 
 * 驱动命令数据紧跟在请求结构体之后,所以加上请求大小即可得到PDU
 */
static inline void *blk_mq_rq_to_pdu(struct request *rq)
{
	return rq + 1;
}

/* 遍历队列的所有硬件上下文 */
#define queue_for_each_hw_ctx(q, hctx, i)				\
	xa_for_each(&(q)->hctx_table, (i), (hctx))

/* 遍历硬件上下文的所有软件上下文 */
#define hctx_for_each_ctx(hctx, ctx, i)					\
	for ((i) = 0; (i) < (hctx)->nr_ctx &&				\
	     ({ ctx = (hctx)->ctxs[(i)]; 1; }); (i)++)

/* 清理请求 */
static inline void blk_mq_cleanup_rq(struct request *rq)
{
	if (rq->q->mq_ops->cleanup_rq)
		rq->q->mq_ops->cleanup_rq(rq);
}

/* 设置硬件上下文的刷新队列锁类 */
void blk_mq_hctx_set_fq_lock_class(struct blk_mq_hw_ctx *hctx,
		struct lock_class_key *key);

/* 检查是否是同步请求 */
static inline bool rq_is_sync(struct request *rq)
{
	return op_is_sync(rq->cmd_flags);
}

/* 初始化请求 */
void blk_rq_init(struct request_queue *q, struct request *rq);

/* 准备克隆请求 */
int blk_rq_prep_clone(struct request *rq, struct request *rq_src,
		struct bio_set *bs, gfp_t gfp_mask,
		int (*bio_ctr)(struct bio *, struct bio *, void *), void *data);

/* 取消准备克隆请求 */
void blk_rq_unprep_clone(struct request *rq);

/* 插入克隆的请求 */
blk_status_t blk_insert_cloned_request(struct request *rq);

/* 请求映射数据结构体 */
struct rq_map_data {
	struct page **pages;           /* 页面指针数组 */
	unsigned long offset;          /* 偏移量 */
	unsigned short page_order;     /* 页面阶数 */
	unsigned short nr_entries;     /* 条目数 */
	bool null_mapped;              /* 是否空映射 */
	bool from_user;                /* 是否来自用户空间 */
};

/* 映射用户空间数据到请求 */
int blk_rq_map_user(struct request_queue *, struct request *,
		struct rq_map_data *, void __user *, unsigned long, gfp_t);

/* 映射用户空间IO数据到请求 */
int blk_rq_map_user_io(struct request *, struct rq_map_data *,
		void __user *, unsigned long, gfp_t, bool, int, bool, int);

/* 映射用户空间IO向量到请求 */
int blk_rq_map_user_iov(struct request_queue *, struct request *,
		struct rq_map_data *, const struct iov_iter *, gfp_t);

/* 取消映射用户空间数据 */
int blk_rq_unmap_user(struct bio *);

/* 映射内核空间数据到请求 */
int blk_rq_map_kern(struct request_queue *, struct request *, void *,
		unsigned int, gfp_t);

/* 追加bio到请求 */
int blk_rq_append_bio(struct request *rq, struct bio *bio);

/* 立即执行请求 */
void blk_execute_rq_nowait(struct request *rq, bool at_head);

/* 执行请求 */
blk_status_t blk_execute_rq(struct request *rq, bool at_head);

/* 检查是否是轮询请求 */
bool blk_rq_is_poll(struct request *rq);

/* 请求迭代器结构体 */
struct req_iterator {
	struct bvec_iter iter;  /* bio向量迭代器 */
	struct bio *bio;        /* 当前bio */
};

/* 遍历请求的所有bio */
#define __rq_for_each_bio(_bio, rq)	\
	if ((rq->bio))			\
		for (_bio = (rq)->bio; _bio; _bio = _bio->bi_next)

/* 遍历请求的所有段 */
#define rq_for_each_segment(bvl, _rq, _iter)			\
	__rq_for_each_bio(_iter.bio, _rq)			\
		bio_for_each_segment(bvl, _iter.bio, _iter.iter)

/* 遍历请求的所有bio向量 */
#define rq_for_each_bvec(bvl, _rq, _iter)			\
	__rq_for_each_bio(_iter.bio, _rq)			\
		bio_for_each_bvec(bvl, _iter.bio, _iter.iter)

/* 检查是否是最后一个bio向量 */
#define rq_iter_last(bvec, _iter)				\
		(_iter.bio->bi_next == NULL &&			\
		 bio_iter_last(bvec, _iter.iter))

/* 获取请求的当前扇区位置 */
static inline sector_t blk_rq_pos(const struct request *rq)
{
	return rq->__sector;
}

/* 获取请求的总字节数 */
static inline unsigned int blk_rq_bytes(const struct request *rq)
{
	return rq->__data_len;
}

/* 获取请求当前段的字节数 */
static inline int blk_rq_cur_bytes(const struct request *rq)
{
	if (!rq->bio)
		return 0;
	if (!bio_has_data(rq->bio))	/* 无数据请求(如discard) */
		return rq->bio->bi_iter.bi_size;
	return bio_iovec(rq->bio).bv_len;
}

/* 获取请求的总扇区数 */
static inline unsigned int blk_rq_sectors(const struct request *rq)
{
	return blk_rq_bytes(rq) >> SECTOR_SHIFT;
}

/* 获取请求当前段的扇区数 */
static inline unsigned int blk_rq_cur_sectors(const struct request *rq)
{
	return blk_rq_cur_bytes(rq) >> SECTOR_SHIFT;
}

/* 获取请求的统计扇区数 */
static inline unsigned int blk_rq_stats_sectors(const struct request *rq)
{
	return rq->stats_sectors;
}

/* 获取请求的有效载荷字节数
 * 对于像WRITE SAME这样的命令,其有效载荷大小可能与请求大小不同
 * 使用RQF_SPECIAL_PAYLOAD标志的驱动需要使用此函数计算数据传输大小
 */
static inline unsigned int blk_rq_payload_bytes(struct request *rq)
{
	if (rq->rq_flags & RQF_SPECIAL_PAYLOAD)
		return rq->special_vec.bv_len;
	return blk_rq_bytes(rq);
}

/* 获取请求的第一个完整bio向量
 * 调用者需要确保有bio向量存在
 */
static inline struct bio_vec req_bvec(struct request *rq)
{
	if (rq->rq_flags & RQF_SPECIAL_PAYLOAD)
		return rq->special_vec;
	return mp_bvec_iter_bvec(rq->bio->bi_io_vec, rq->bio->bi_iter);
}

/* 获取请求的bio数量 */
static inline unsigned int blk_rq_count_bios(struct request *rq)
{
	unsigned int nr_bios = 0;
	struct bio *bio;

	__rq_for_each_bio(bio, rq)
		nr_bios++;

	return nr_bios;
}

/* 窃取请求的bio */
void blk_steal_bios(struct bio_list *list, struct request *rq);

/* 更新请求
 * 完成指定字节数并更新请求,但不标记为完成
 */
bool blk_update_request(struct request *rq, blk_status_t error,
			       unsigned int nr_bytes);

/* 中止请求 */
void blk_abort_request(struct request *);

/* 获取请求的物理段数
 * 通常是提交者发送的不连续数据段数
 * 但对于无数据命令(如discard),可能没有实际数据段
 * 此时返回1以便映射特殊负载
 */
static inline unsigned short blk_rq_nr_phys_segments(struct request *rq)
{
	if (rq->rq_flags & RQF_SPECIAL_PAYLOAD)
		return 1;
	return rq->nr_phys_segments;
}

/* 获取请求的discard段数
 * 每个合并到请求中的discard bio都计为一个段
 */
static inline unsigned short blk_rq_nr_discard_segments(struct request *rq)
{
	return max_t(unsigned short, rq->nr_phys_segments, 1);
}

/* 映射请求到散列表 */
int __blk_rq_map_sg(struct request_queue *q, struct request *rq,
		struct scatterlist *sglist, struct scatterlist **last_sg);

/* 映射请求到散列表(简化版) */
static inline int blk_rq_map_sg(struct request_queue *q, struct request *rq,
		struct scatterlist *sglist)
{
	struct scatterlist *last_sg = NULL;

	return __blk_rq_map_sg(q, rq, sglist, &last_sg);
}

/* 打印请求标志 */
void blk_dump_rq_flags(struct request *, char *);

#endif /* BLK_MQ_H */