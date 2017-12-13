/* OX: OpenChannel NVM Express SSD Controller
 *
 * Copyright (C) 2016, IT University of Copenhagen. All rights reserved.
 * Written by Ivan Luiz Picoli <ivpi@itu.dk>
 *
 * Funding support provided by CAPES Foundation, Ministry of Education
 * of Brazil, Brasilia - DF 70040-020, Brazil.
 *
 * This code is licensed under the GNU GPL v2 or later.
 */

#ifndef APPNVM_H
#define APPNVM_H

#include <sys/queue.h>
#include "hw/block/ox-ctrl/include/ssd.h"

#define APP_IO_RETRY       0

#define APP_RSV_BBT_OFF    0
#define APP_RSV_META_OFF   1
#define APP_RSV_L2P_OFF    2
#define APP_RSV_BLK_COUNT  APP_RSV_BBT_OFF + APP_RSV_META_OFF + APP_RSV_L2P_OFF;

#define APP_MAGIC          0x3c

enum {
    FTL_PGMAP_OFF   = 0,
    FTL_PGMAP_ON    = 1
};

enum app_bbt_state {
    NVM_BBT_FREE = 0x0, // Block is free AKA good
    NVM_BBT_BAD  = 0x1, // Block is bad
    NVM_BBT_GBAD = 0x2, // Block has grown bad
    NVM_BBT_DMRK = 0x4, // Block has been marked by device side
    NVM_BBT_HMRK = 0x8  // Block has been marked by host side
};

#define APP_BBT_EMERGENCY   0x0 // Creates the bbt without erasing the channel
#define APP_BBT_ERASE       0x1 // Checks for bad blocks only erasing the block
#define APP_BBT_FULL        0x2 // Checks for bad blocks erasing the block,
                                 //   writing and reading all pages,
                                 //   and comparing the buffers

struct app_l2p_entry {
    uint64_t laddr;
    uint64_t paddr;
};

struct app_page {

};

struct app_bbtbl {
    uint8_t  magic;
    uint32_t bb_sz;
    uint32_t bb_count;
    /* This struct is stored on NVM up to this point, *tbl is not stored */
    uint8_t  *tbl;
};

struct app_blk_md_entry {
    uint32_t    erase_count;
    uint32_t    current_pg;
    uint8_t     pg_state[64]; /* maximum of 512 pages per blk */
} __attribute__((packed)) ;   /* 72 bytes per entry */

struct app_blk_md {
    uint8_t  magic;
    uint32_t entries;
    /* This struct is stored on NVM up to this point, *tbl is not stored */
    uint8_t  *tbl;
};

struct app_channel {
    struct nvm_channel      *ch;
    struct app_bbtbl        *bbtbl;
    struct app_blk_md       *blk_md;
    uint16_t                bbt_blk;  /* Rsvd blk ID for bad block table */
    uint16_t                meta_blk; /* Rsvd blk ID for block metadata */
    uint16_t                l2p_blk;  /* Rsvd blk ID for l2p metadata */
    LIST_ENTRY(app_channel) entry;
};

typedef int (app_bbt_create)(struct app_channel *, struct app_bbtbl *, uint8_t);
typedef int (app_bbt_flush) (struct app_channel *, struct app_bbtbl *);
typedef int (app_bbt_load) (struct app_channel *, struct app_bbtbl *);

typedef int (app_md_create)(struct app_channel *, struct app_blk_md *);
typedef int (app_md_flush) (struct app_channel *, struct app_blk_md *);
typedef int (app_md_load) (struct app_channel *, struct app_blk_md *);

struct app_global_bbt {
    app_bbt_create      *create_fn;
    app_bbt_flush       *flush_fn;
    app_bbt_load        *load_fn;
};

struct app_global_md {
    app_md_create      *create_fn;
    app_md_flush       *flush_fn;
    app_md_load        *load_fn;
};

struct app_global {
    struct app_global_bbt   bbt;
    struct app_global_md    md;
};

int app_io_rsv_blk (struct app_channel *lch, uint8_t cmdtype,
                                     void **buf_vec, uint16_t blk, uint16_t pg);

struct app_global *appnvm (void);
void bbt_byte_register (void);
void blk_md_register (void);

#endif /* APP_H */