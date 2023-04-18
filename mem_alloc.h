#pragma once

#include	"hp_structs.h"

struct frame_t {
	size_t n;
	void *physaddr;
	void *virtaddr;
	//float *a[CH];
	//float *b[CH];
	root_info_t *root_info;
	block_info_t *block_info;
};

struct adc_card_t
{
	static double adc_rate;
	static int chans;
	static int samples;
	static int pages;
	static unsigned int block_num;
	static unsigned int block_size;
	static int blk_in_page;
	uint16_t dma_chan;
	char fpath[MAX_PATH];
	CXdmaDevice* pAdc;
	int bus;
	int dev;
	void* datas;
	vector<frame_t> frames;
	uint32_t sec;
	uint32_t nsec;

};

#define DATA_EMPTY      0   // данных нет
#define DATA_BUSY       1   // фрейм в процессе заполнения
#define DATA_PRESENT    2   // фрейм с данными
#define DATA_ERROR      3   // была ошибка при приеме фрейма
#define DATA_DONTUSE    4   // метка для не используемых фреймов, обычно на последней странице

void* alloc_c2h_buffers(struct adc_card_t *pDev, int ndev);
//void* alloc_c2h_buffers(CXdmaDevice *pDev, const char* fname, int ndev, int chans, int samples, int page_num,
//	uint32_t blk_size, uint32_t blk_num, uint16_t dma_chan);
void free_buffers(void *buf);

void set_block_info(struct frame_t *frame, uint32_t iframe, int32_t sec, int32_t nsec);
void* get_block_info(struct frame_t *frame);
void print_block_info(struct frame_t *frame, int idx_blk);


